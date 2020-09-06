
#ifndef __MM__
#define __MM__
#define MM_MAX_STRUCT_NAME_SIZE 64

#define MAX_FAMILIES_PER_VM_PAGE \
    (SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t *))/(sizeof(vm_page_family_t))

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr,current)                \
{                                                                                    \
    uint32_t count = 0;                                                               \
    for(current = vm_page_for_families_ptr->vm_page_family;                          \
        current->struct_size && count < MAX_FAMILIES_PER_VM_PAGE;current++,count++){ \
    
#define ITERATE_PAGE_FAMILIES_END }}

typedef struct  vm_page_family_{
    char struct_name[MM_MAX_STRUCT_NAME_SIZE];
    uint32_t struct_size;
}vm_page_family_t;

typedef struct vm_page_for_families{
    struct vm_page_for_families *next;  // pointer to the next page
    vm_page_family_t *vm_page_family;   //pointer to the array that stores the struct data
}vm_page_for_families_t;

#endif
