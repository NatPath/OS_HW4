#include <unistd.h>
void* smalloc(size_t size){
    if(size == 0 || size > 10^8){
        return NULL;
    }
    void* res=sbrk(size);
    if(res == (void*)(-1)){
        return NULL;
    }
    res=res+1;
    return res;
}