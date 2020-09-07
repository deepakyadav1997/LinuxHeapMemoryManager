//testing our functions
#include<stdio.h>
#include "uapi_mm.h"

typedef struct employee{
   char name[64];
   uint32_t emp_number;
}emp_t;

typedef struct student{
    char name[64];
    uint32_t roll_number;
    uint32_t marks_physics;
    uint32_t marks_chemistry;
    uint32_t marks_maths;
}student_t;

int main(int argc,char** argv){
    mm_init();
    // printf("System Page size = %d\n",SYSTEM_PAGE_SIZE);
    // void* address1 = mm_get_new_vm_page_from_kernel(1);
    // void* address2 = mm_get_new_vm_page_from_kernel(1);
    // printf("Address 1 = %p \t Address 2 = %p\n",address1,address2);
    // return 0;
    MM_REGISTER_STRUCT(emp_t);
    MM_REGISTER_STRUCT(student_t);
    mm_print_registered_page_families();
    char* struct_to_find = "avc";
    if(lookup_page_family_by_name(struct_to_find)){
        printf("%s is found\n",struct_to_find);
    }
    else{
        printf("%s is not found\n",struct_to_find);
    }
    return 0;
}