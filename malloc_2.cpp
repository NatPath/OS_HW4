#include <unistd.h>
#include <stdlib.h>

class MetaData{
    size_t  _size;
    bool _is_free;
    void* _data_block;
    MetaData* _next;
    MetaData* _prev;
    public:
    MetaData(size_t size ,bool is_free, MetaData* last_metadata):_size(size),_is_free(is_free),_prev(last_metadata),_next(nullptr){
    }
    size_t getSize(){
        return _size;
    }
    bool isFree(){
        return _is_free;
    }
    bool setFree(bool new_free_status){
        _is_free=new_free_status;        
    }


};


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
    void* sbrk_res;
    if (heap_bottom){
        sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
        if (sbrk_res==NULL){
            return NULL;
        }
        heap_bottom=(MetaData*)sbrk_res;
        return heap_bottom+sizeof(MetaData);
    }
    MetaData* itt= heap_bottom;
    while (itt){
        if (itt->isFree()&& itt->getSize()>=size){
            sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
            if (sbrk_res==NULL){
                return NULL;
            }
            heap_bottom=(MetaData*)sbrk_res;
            return heap_bottom+sizeof(MetaData);
        }        
    }
}

void* scalloc(size_t num, size_t size){
    void* smalloc_res=smalloc(size);
}
void* sfree(void* p){

}
void* srealloc(void* oldp, size_t size){

}
