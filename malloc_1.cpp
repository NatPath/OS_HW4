#include <unistd.h>
#include <iostream>
#include "malloc_1.h"

#define HUNDRED_MIL 100000000
void* smalloc(size_t size){
    if(size == 0 || size > HUNDRED_MIL){
        return NULL;
    }
    void* res=sbrk(size);
    if(res == (void*)(-1)){
        return NULL;
    }
    res=(void*)((long)res+1);
    return res;
}