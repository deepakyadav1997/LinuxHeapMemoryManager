#include<stdio.h>
#include<memory.h>
#include<unistd.h>  // for getpagesize
#include<sys/mman.h>    //for mmap()

static size_t SYSTEM_PAGE_SIZE = 0;

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
        printf("Error! Vittual memory page allocation failed.\n");
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
int main(int argc,char** argv){
    mm_init();
    printf("System Page size = %d\n",SYSTEM_PAGE_SIZE);
    void* address1 = mm_get_new_vm_page_from_kernel(1);
    void* address2 = mm_get_new_vm_page_from_kernel(1);
    printf("Address 1 = %p \t Address 2 = %p\n",address1,address2);
    return 0;
}