#include <unistd.h>

#define HUNDRED_MIL 100000000
void* smalloc(size_t size){
    if(size == 0 || size > HUNDRED_MIL){
        return NULL;
    }
    void* res=sbrk(size);
    if(res == (void*)(-1)){
        return NULL;
    }
    return res;
}