#include<stdio.h>
#include<memory.h>
#include<stdint.h>
#include<unistd.h>  // for getpagesize
#include<sys/mman.h>    //for mmap()
#include<assert.h>
#include "mm.h"
#include "glthreads_lib/glthread.h"

static size_t SYSTEM_PAGE_SIZE = 0;
static vm_page_for_families_t *first_vm_page_for_families = NULL;

void mm_init(){
    SYSTEM_PAGE_SIZE = getpagesize(); // from unistd.h
}

//Function to request Virtual Memory page from the kernel
static void* mm_get_new_vm_page_from_kernel(int units){
    char* vm_page =  mmap(
            0,
            units*SYSTEM_PAGE_SIZE,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_ANON | MAP_PRIVATE ,
            0,0);
    if(vm_page == MAP_FAILED){
        printf("Error! Virtual memory page allocation failed.\n");
        return NULL;
    }
    memset(vm_page,0,units * SYSTEM_PAGE_SIZE);
    return (void*) vm_page;
}
//Function to return the vm pages back to the kernel
static void mm_return_vm_page_to_kernel(void* vm_page,int units){
    if(munmap(vm_page,units*SYSTEM_PAGE_SIZE)){ // if it return 0;
        printf("Error! Could not munmap the given virtual memory pages\n");
    }
}
void mm_instantiate_new_page_family(char* struct_name, uint32_t size){
    vm_page_family_t *vm_page_family_current = NULL;
    vm_page_for_families_t *new_vm_page_for_families = NULL;
    if(size > SYSTEM_PAGE_SIZE){
        printf("Error! Structure %s of size %d size greater than system page size\n",struct_name,size);
        return;
    }
    if(!first_vm_page_for_families){ // First vm page is null,request new page.
        first_vm_page_for_families = (vm_page_for_families_t*)mm_get_new_vm_page_from_kernel(1);
        first_vm_page_for_families->next = NULL;
        strncpy(first_vm_page_for_families->vm_page_family[0].struct_name,
                struct_name,MM_MAX_STRUCT_NAME_SIZE
        );
        first_vm_page_for_families->vm_page_family[0].struct_size = size;
        init_glthread(&first_vm_page_for_families->vm_page_family[0].free_blocks_queue_head); //initializing head of the list
        return;
    }
    uint32_t page_family_count = 0;
    ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families,vm_page_family_current){
        if(strncmp(vm_page_family_current->struct_name,
           struct_name,MM_MAX_STRUCT_NAME_SIZE) != 0){
               page_family_count++;
               continue;
           }
           assert(0);
    } ITERATE_PAGE_FAMILIES_END;
    if(page_family_count == MAX_FAMILIES_PER_VM_PAGE){ //Current page is full
        new_vm_page_for_families = (vm_page_for_families_t*)mm_get_new_vm_page_from_kernel(1);
        new_vm_page_for_families->next = first_vm_page_for_families;
        first_vm_page_for_families = new_vm_page_for_families;
        strncpy(first_vm_page_for_families->vm_page_family[0].struct_name,
                struct_name,MM_MAX_STRUCT_NAME_SIZE
        );
        first_vm_page_for_families->vm_page_family[0].struct_size = size;
        init_glthread(&first_vm_page_for_families->vm_page_family[0].free_blocks_queue_head);
        return;
    }
    else{ // copy the data to current pointer of the looping macro as it will 
          //point to the next blank place in the page
        strncpy(vm_page_family_current->struct_name,
                struct_name,MM_MAX_STRUCT_NAME_SIZE
        );
        vm_page_family_current->struct_size = size;
        init_glthread(&vm_page_family_current->free_blocks_queue_head);
        return;
    }
}

vm_page_family_t * lookup_page_family_by_name(char *struct_name){
     vm_page_for_families_t *current_page = first_vm_page_for_families;
    vm_page_family_t *current_family = NULL;
    while(current_page){
        ITERATE_PAGE_FAMILIES_BEGIN(current_page,current_family){
            if(strcmp(current_family->struct_name,struct_name) == 0){
                return current_family;
            }
        }ITERATE_PAGE_FAMILIES_END;
        current_page = current_page->next;
    } 
    return NULL;  
}
static void mm_union_free_blocks(block_meta_data_t* first,block_meta_data_t* second){
    if(first->is_free != MM_TRUE || second->is_free != MM_TRUE){
        printf("Error! Some blocks being merged are not free\n");
        assert(0);
    }
    first->next = second->next;
    if(second->next){
        first->next->previous = first;
    }
    first->block_size += second->block_size+sizeof(block_meta_data_t);
}
vm_bool_t mm_is_vm_page_empty(vm_page_t* vm_page){
    if(vm_page->block_meta_data.is_free == MM_TRUE
       && vm_page->block_meta_data.next == NULL
       && vm_page->block_meta_data.previous == NULL){
           return MM_TRUE;
       }
       return MM_FALSE;
}

static inline uint32_t mm_max_page_allocatable_memory(int units){
    return (uint32_t)((SYSTEM_PAGE_SIZE*units)-offset_of(vm_page_t,page_memory));
}

vm_page_t* allocate_vm_page(vm_page_family_t* vm_page_family){
    vm_page_t* new_vm_page = (vm_page_t*)mm_get_new_vm_page_from_kernel(1);
    /*Initialize the new page*/
    MARK_VM_PAGE_EMPTY(new_vm_page);
    new_vm_page->block_meta_data.block_size = mm_max_page_allocatable_memory(1);
    new_vm_page->block_meta_data.offset = offset_of(vm_page_t,block_meta_data);
    new_vm_page->previous = NULL;
    new_vm_page->next = NULL;
    new_vm_page->page_family = vm_page_family;
    init_glthread(&new_vm_page->block_meta_data.priority_queue_node);
    if(vm_page_family->first_page != NULL){
        new_vm_page->next = vm_page_family->first_page;
        vm_page_family->first_page->previous = new_vm_page;
    }
    vm_page_family->first_page = new_vm_page;;
    return new_vm_page;
}
void mm_vm_page_delete_and_free(vm_page_t* vm_page){
    vm_page_t* previous = vm_page->previous;
    vm_page_t* next = vm_page->next;
    vm_page_family_t *page_family = vm_page->page_family;
    if(page_family->first_page == vm_page){
        page_family->first_page = page_family->first_page->next;
    }
    if(previous != NULL){
        previous->next = next;
    }
    if(next != NULL){
        next->previous = previous;
    }
    mm_return_vm_page_to_kernel((void*)vm_page,1);
}

static int blocks_comparision_function(void* block1,void* block2){
    block_meta_data_t* meta_block1 = (block_meta_data_t*) block1;
    block_meta_data_t* meta_block2 = (block_meta_data_t*) block2;
    return meta_block1->block_size - meta_block2->block_size;
}

static void add_free_meta_data_block_to_free_block_list(vm_page_family_t* vm_page_family,
                                                        block_meta_data_t* free_block){
    assert(free_block->is_free = MM_TRUE);
    glthread_priority_insert(&vm_page_family->free_blocks_queue_head,
                            &free_block->priority_queue_node,
                            blocks_comparision_function,
                            offset_of(block_meta_data_t,priority_queue_node));                                  
}

static inline block_meta_data_t* mm_get_biggest_free_block_page_family(vm_page_family_t* vm_page_family){
    // will return either the first block,i.e largest or null if empty
    if(vm_page_family->free_blocks_queue_head.right)
        return glthread_to_block_meta_data(vm_page_family->free_blocks_queue_head.right);
    else 
        return NULL;
}

static vm_bool_t mm_split_free_data_block_for_allocation(vm_page_family_t* vm_page_family,
                                                        block_meta_data_t* block,
                                                        uint32_t size){
                                                    
    block_meta_data_t* new_block = NULL;
    assert(block->is_free == MM_TRUE);
    if(block->block_size < size){
        return MM_FALSE;
    }
    uint32_t remaining_size = block->block_size - size;
    block->is_free = MM_FALSE;
    block->block_size = size;
    remove_glthread(&block->priority_queue_node);

    // Case 1: No split
    if(remaining_size == 0){
        return MM_TRUE;
    }
    // Case 3: partial internal fragmentation, soft spilt
    if(sizeof(block_meta_data_t) < remaining_size 
            && remaining_size < sizeof(block_meta_data_t) + vm_page_family->struct_size){
        // New metablock
        new_block = NEXT_META_BLOCK_BY_SIZE(block);
        new_block->block_size = remaining_size - sizeof(block_meta_data_t);
        new_block->is_free = MM_TRUE;
        new_block->next = block->next;
        new_block->previous = block;
        block->next = new_block;
        if(new_block->next){
            new_block->next->previous = new_block;
        }
        new_block->offset = block->offset + sizeof(block_meta_data_t) + block->block_size;
        add_free_meta_data_block_to_free_block_list(vm_page_family,new_block);
        
    }   
    //case 3: hard split
    else if(sizeof(block_meta_data_t) > remaining_size){
        // Do nothing yet. Not sure about how this block wil be merged while freeing
    }
    //case 2: full split 
    else{
        // Exactly same as sodt split. Look for refactoring opportunities
        new_block = NEXT_META_BLOCK_BY_SIZE(block);
        new_block->block_size = remaining_size - sizeof(block_meta_data_t);
        new_block->is_free = MM_TRUE;
        new_block->next = block->next;
        new_block->previous = block;
        block->next = new_block;
        if(new_block->next){
            new_block->next->previous = new_block;
        }
        new_block->offset = block->offset + sizeof(block_meta_data_t) + block->block_size;
        add_free_meta_data_block_to_free_block_list(vm_page_family,new_block);
    }
    return MM_TRUE;
}


static block_meta_data_t* mm_allocate_free_data_block(vm_page_family_t* vm_page_family,int size){
    vm_bool_t status = MM_FALSE;
    vm_page_t * vm_page = NULL;
    block_meta_data_t* largest_free_block = mm_get_biggest_free_block_page_family(vm_page_family);
    if(!largest_free_block || largest_free_block->block_size < size){ // add new vm page
        vm_page = allocate_vm_page(vm_page_family);
        // Allocate free memory now
        status = mm_split_free_data_block_for_allocation(vm_page_family,
                                                        &vm_page->block_meta_data,size);
        if(status = MM_TRUE){
            return &vm_page->block_meta_data;
        }
        return NULL;
    }
    // Biggest existing block can satisfy the need of the application.
    if(largest_free_block){
        status = mm_split_free_data_block_for_allocation(vm_page_family,
                                                        largest_free_block,size);
    }
    if(status = MM_TRUE){
        return largest_free_block;
    }
    return NULL;
}


void* xcalloc(char* struct_name,uint32_t units){
    vm_page_family_t* vm_page_family = lookup_page_family_by_name(struct_name);
    if(vm_page_family == NULL){
        printf("Error!Structure %s is not registered with the memory manager\n",struct_name);
        return NULL;
    }
    if(units*vm_page_family->struct_size > mm_max_page_allocatable_memory(1)){
        printf("Error! Memory required is more than page size\n");
        return NULL;
    }
    block_meta_data_t*  free_block_meta_data = NULL;
    free_block_meta_data = mm_allocate_free_data_block(vm_page_family,units*vm_page_family->struct_size);
    if(free_block_meta_data){
        memset((char*)(free_block_meta_data+1),0,free_block_meta_data->block_size);
        return (void*)(free_block_meta_data+1);
    }
    return NULL;
}

void print_page_family_details(vm_page_family_t* vm_page_family){
    //printf("Structure: %s    Size: %d\n",vm_page_family->struct_name,vm_page_family->struct_size);
    vm_page_t* current_page = NULL;
    ITERATE_VM_PAGE_BEGIN(vm_page_family,current_page){
        printf("\tprevious = %x  next = %x\n",current_page->previous,current_page->next);
        printf("\t\t%-20s %-10s %-10s %-10s %-6s %-20s %-20s\n","Address","Block No.","Allocated?","block_size","offset","previous","next");
        block_meta_data_t* current_block = NULL;
        int block_count = 0;
        ITERATE_VM_PAGES_ALL_BLOCKS_BEGIN(current_page,current_block){
            char* is_free = current_block->is_free==MM_TRUE?"Free":"Allocated";
            printf("\t\t%-20x %-10d %-10s %-10d %-6d %-20x %-20x\n",
                    current_block,++block_count,is_free,current_block->block_size,current_block->offset,current_block->previous,current_block->next);
        }ITERATE_VM_PAGES_ALL_BLOCKS_END;
    }ITERATE_VM_PAGE_END;
}
void mm_print_registered_page_families(){
    vm_page_for_families_t *current_page = first_vm_page_for_families;
    vm_page_family_t *current_family = NULL;
    while(current_page){
        ITERATE_PAGE_FAMILIES_BEGIN(current_page,current_family){
            printf("Struct name: %s Struct size: %d\n",current_family->struct_name,
                   current_family->struct_size);
            print_page_family_details(current_family);
        }ITERATE_PAGE_FAMILIES_END;
        current_page = current_page->next;
    }
}