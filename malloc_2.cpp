#include <unistd.h>
#include <stdlib.h>

class MetaData{
    size_t  _size;
    bool _is_free;
    MetaData* _next;
    MetaData* _prev;
    public:
    MetaData(size_t size ,bool is_free, MetaData* last_metadata):_size(size),_is_free(is_free),_prev(last_metadata),_next(nullptr){
        
    }
    size_t getSize(){
        return _size;
    }
};
class BlockData{
    MetaData* _metadata;

}


MetaData *heap_bottom=nullptr;

void* sbrk_wrap(size_t size){
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

void* smalloc(size_t size){
    if (heap_bottom){
        void* sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
        if (sbrk_res==NULL){
            return NULL;
        }
        heap_bottom=(MetaData*)sbrk_res;
        return heap_bottom+sizeof(MetaData);
    }
    
}
void* scalloc(size_t num, size_t size){

}
void* sfree(void* p){

}
void* srealloc(void* oldp, size_t size){

}
