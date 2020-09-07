#ifndef __UAPI_MM__
#define __UAPI_MM__
#include<stdint.h>
#include "mm.h"


#define MM_REGISTER_STRUCT(struct_name) \
    (mm_instantiate_new_page_family(#struct_name,sizeof(struct_name)))

void mm_init();
void mm_instantiate_new_page_family(char* struct_name, uint32_t size);
void mm_print_registered_page_families();
vm_page_family_t  *lookup_page_family_by_name(char *struct_name);
#endif