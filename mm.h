
#ifndef __MM__
#define __MM__
#define MM_MAX_STRUCT_NAME_SIZE 64

typedef enum{

    MM_FALSE,
    MM_TRUE
} vm_bool_t;

#define MAX_FAMILIES_PER_VM_PAGE \
    (SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t *))/(sizeof(vm_page_family_t))

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr,current)                 \
{                                                                                     \
    uint32_t count = 0;                                                               \
    for(current = vm_page_for_families_ptr->vm_page_family;                           \
        current->struct_size && count < MAX_FAMILIES_PER_VM_PAGE;current++,count++){  \
    
#define ITERATE_PAGE_FAMILIES_END }}

#define offset_of(container_structure, field_name)  &(((container_structure *)NULL)->field_name)

#define MM_GET_PAGE_FROM_META_BLOCK(block_meta_data_ptr)    \
    (void*)((char*)block_meta_data_ptr - block_meta_data_ptr->offset)

#define NEXT_META_BLOCK(block_meta_data_ptr)    \
    block_meta_data_ptr->next

#define NEXT_META_BLOCK_BY_SIZE(block_meta_data_ptr) \
    (block_meta_data_t*)((char*)(block_meta_data_ptr + 1) + block_meta_data_ptr->block_size)

#define PREV_META_BLOCK(block_meta_data_ptr) \
    block_meta_data_ptr->previous

#define mm_bind_blocks_for_allocation(allocated_meta_block, free_meta_block) \
    free_meta_block->next = allocated_meta_block->next;                      \
    free_meta_block->previous = allocated_meta_block;                        \
    allocated_meta_block->next = free_meta_block;                            \
    if(free_meta_block->next != NULL){                                       \
        free_meta_block->next->previous = free_meta_block;                   \
    }
    

typedef struct  vm_page_family_{
    char struct_name[MM_MAX_STRUCT_NAME_SIZE];
    uint32_t struct_size;
}vm_page_family_t;

typedef struct vm_page_for_families{
    struct vm_page_for_families *next;  // pointer to the next page
    vm_page_family_t vm_page_family[0];   //pointer to the array that stores the struct data
}vm_page_for_families_t;

typedef struct block_meta_data{
    vm_bool_t is_free;
    uint32_t block_size;
    struct block_meta_data *next;
    struct block_meta_data *previous;
    uint32_t offset;
}block_meta_data_t;

#endif
