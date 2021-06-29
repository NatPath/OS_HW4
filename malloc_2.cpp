#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include "malloc_2.h"

 struct statistics{

    size_t _num_free_blocks;
    size_t _num_free_bytes;
    size_t  _num_allocated_blocks;
    size_t _num_allocated_bytes;

} stats_default{0,0,0,0};

typedef struct statistics* Stats;


class MetaData{
    size_t  _size;
    bool _is_free;
    //void* _data_block;
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
    MetaData* getNext(){
        return _next;
    }
    void setNext(MetaData* next){
        _next=next;
    }
    MetaData* getPrev(){
        return _prev;
    }
    void setPrev(MetaData* prev){
        _prev=prev;
    }


};

MetaData *heap_bottom=nullptr;
statistics stats = stats_default;
void stats_allocate_block(size_t num_bytes){
     stats._num_allocated_blocks++;
     stats._num_allocated_bytes+=num_bytes;
}


//increses the heap size by size (allocates a block). returns the address of the new allocated block
#define HUNDRED_MIL 100000000
void* sbrk_wrap(size_t size){
    if(size == sizeof(MetaData) || size > HUNDRED_MIL + sizeof(MetaData)){
        return NULL;
    }
    void* res=sbrk(size);
    if(res == (void*)(-1)){
        return NULL;
    }
    res=(void*)((char*)res+1);
    return res;
}

void* smalloc(size_t size){
    void* sbrk_res;
    if (!heap_bottom){
        sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
        if (sbrk_res==NULL){
            return NULL;
        }
        heap_bottom=(MetaData*)sbrk_res;
        *heap_bottom = MetaData(size,0,NULL);
        stats_allocate_block(size);
        return heap_bottom+sizeof(MetaData);
    }
    MetaData* itt= heap_bottom;
    while (itt){
        if (itt->isFree()&& itt->getSize()>=size){
            itt->setFree(0);
            stats._num_free_blocks--;
            stats._num_free_bytes-=size;
            return itt+sizeof(MetaData);
        }        
        else{
            if (itt->getNext()==NULL){// checked the last block, need to allocate a new one
                sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
                if (sbrk_res==NULL){
                    return NULL;
                }
                itt->setNext((MetaData*)sbrk_res);
                *(itt->getNext()) = MetaData(size,0,itt);
                stats_allocate_block(size);
                return itt->getNext()+sizeof(MetaData);
            }
            itt = itt->getNext();
        }
    }
}

void* scalloc(size_t num, size_t size){
    void* smalloc_res=smalloc(size * num);
    if(smalloc_res == NULL){
        return NULL;
    }

    std::memset(smalloc_res,0,size*num);
    return smalloc_res;
}
void sfree(void* p){
    if(p == NULL){
        return;
    }

    MetaData* meta =(MetaData*)((char*)p-sizeof(MetaData));
    stats._num_free_blocks++;
    stats._num_free_bytes+=meta->getSize();
    meta->setFree(true);
    
    
    
}
void* srealloc(void* oldp, size_t size){
    MetaData* meta =(MetaData*)((char*)oldp-sizeof(MetaData));
    if (meta->getSize()> size){
        return oldp;       
    }
    if (oldp==NULL){
        return (MetaData*)smalloc(size);
    }
    else{
        void* smalloc_res=smalloc(size);
        if (smalloc_res==NULL){
            return NULL;
        }
        MetaData* newp= (MetaData*)smalloc_res;
        std::memcpy(oldp,newp,meta->getSize());
        sfree(oldp);
        return newp;
    }        
}

size_t _num_free_blocks(){
    return stats._num_free_blocks;
}

size_t _num_free_bytes(){
    return stats._num_free_bytes;
}

size_t _num_allocated_blocks(){
    return stats._num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return stats._num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return stats._num_allocated_blocks * sizeof(MetaData);
}

size_t _size_meta_data(){
    return sizeof(MetaData);
}
