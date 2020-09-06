#include<stdio.h>
#include<memory.h>
#include<stdint.h>
#include<unistd.h>  // for getpagesize
#include<sys/mman.h>    //for mmap()
#include<assert.h>
#include "mm.h"

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
        strncpy(first_vm_page_for_families->vm_page_family->struct_name,
                struct_name,MM_MAX_STRUCT_NAME_SIZE
        );
        first_vm_page_for_families->vm_page_family->struct_size = size;
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
        strncpy(first_vm_page_for_families->vm_page_family->struct_name,
                struct_name,MM_MAX_STRUCT_NAME_SIZE
        );
        first_vm_page_for_families->vm_page_family->struct_size = size;
        return;
    }
    else{ // copy the data to current pointer of the looping macro as it will point to the next blank place in the page
        strncpy(vm_page_family_current->struct_name,
                struct_name,MM_MAX_STRUCT_NAME_SIZE
        );
        vm_page_family_current->struct_size = size;
        return;
    }
}
//testing our functions
int main(int argc,char** argv){
    mm_init();
    printf("System Page size = %d\n",SYSTEM_PAGE_SIZE);
    void* address1 = mm_get_new_vm_page_from_kernel(1);
    void* address2 = mm_get_new_vm_page_from_kernel(1);
    printf("Address 1 = %p \t Address 2 = %p\n",address1,address2);
    return 0;
}