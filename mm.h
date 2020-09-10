
#ifndef __MM__
#define __MM__
#define MM_MAX_STRUCT_NAME_SIZE 64
#include "glthreads_lib/glthread.h"

typedef enum{
    MM_FALSE,
    MM_TRUE
} vm_bool_t;

struct vm_page;

#define MAX_FAMILIES_PER_VM_PAGE                                                      \
    (SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t *))/(sizeof(vm_page_family_t))

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr,current)                 \
{                                                                                     \
    uint32_t count = 0;                                                               \
    for(current = vm_page_for_families_ptr->vm_page_family;                           \
        current->struct_size && count < MAX_FAMILIES_PER_VM_PAGE;current++,count++){  \
    
#define ITERATE_PAGE_FAMILIES_END }}

#define ITERATE_VM_PAGE_BEGIN(vm_page_family_ptr,current)                             \
{                                                                                     \
    vm_page_family_t *next = NULL;                                                    \
    for(current = vm_page_family_ptr->first_page;                                     \
        current != null; current = next){                                             \
        next = current->next;

#define ITERATE_VM_PAGE_END }}
#define offset_of(container_structure, field_name)  &(((container_structure *)NULL)->field_name)

#define MM_GET_PAGE_FROM_META_BLOCK(block_meta_data_ptr)                              \
    (void*)((char*)block_meta_data_ptr - block_meta_data_ptr->offset)

#define NEXT_META_BLOCK(block_meta_data_ptr)                                          \
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

#define MARK_VM_PAGE_EMPTY(vm_page_ptr)                                      \
    vm_page_ptr->block_meta_data.is_free == MM_TRUE                          \
    vm_page_ptr->block_meta_data.next == NULL                                \
    vm_page_ptr->block_meta_data.previous == NULL

#define ITERATE_VM_PAGES_ALL_BLOCKS_BEGIN(vm_page_ptr,current)               \
{                                                                            \
    for(block_meta_data_t* current = vm_page_ptr->block_meta_data;           \
        block_meta_data_t *next = NULL;                                      \
        current != null; current = next){                                    \
        next = NEXT_META_BLOCK(current);

 #define ITERATE_VM_PAGES_ALL_BLOCKS_END }}  

typedef struct  vm_page_family_{
    char struct_name[MM_MAX_STRUCT_NAME_SIZE];
    uint32_t struct_size;
    struct vm_page* first_page;
    glthread_t free_blocks_queue_head;
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
    glthread_t priority_queue_node;
    uint32_t offset;
}block_meta_data_t;

GLTHREAD_TO_STRUCT(glthread_to_block_meta_data,block_meta_data_t,glthread_ptr);

typedef struct vm_page{
    struct vm_page* previous;
    struct vm_page* next;
    block_meta_data_t block_meta_data; // Not a pointer but struct itself to allocate memory
    char page_memory[0];  // first data block in VM page
    vm_page_family_t* page_family;    
}vm_page_t;

vm_bool_t mm_is_vm_page_empty(vm_page_t* vm_page);
vm_page_t* allocate_vm_page(vm_page_family_t* vm_page_family);
void mm_vm_page_delete_and_free(vm_page_t* vm_page);

#endif
