#include <unistd.h>
#include <stdlib.h>
#include <cstring>
//#include "malloc_2.h"

// UNCOMMENT THIS BEFORE SUBMITION - BECAUSE WE DONT SUBMIT malloc_2.h
 struct statistics{

    size_t _num_free_blocks;
    size_t _num_free_bytes;
    size_t  _num_allocated_blocks;
    size_t _num_allocated_bytes;

} stats_default{0,0,0,0};

typedef struct statistics* Stats;
Stats stats = &stats_default;


class MetaData{
    size_t  _size;
    bool _is_free;
    void* _data_block;
    MetaData* _next;
    MetaData* _prev;
    public:
    MetaData(size_t size ,bool is_free,void* data_block, MetaData* last_metadata):_size(size),_is_free(is_free),_data_block(data_block),_prev(last_metadata),_next(nullptr){
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
    void* getDataBlock(){
        return _data_block;
    }
};

void stats_allocate_block(size_t num_bytes);
void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

MetaData *heap_bottom=nullptr;

void stats_allocate_block(size_t num_bytes){
     stats->_num_allocated_blocks++;
     stats->_num_allocated_bytes+=num_bytes;
}

void stats_free_block(size_t num_bytes){
    stats->_num_free_blocks++;
    stats->_num_free_bytes+=num_bytes;
}

void stats_unfree_block(size_t num_bytes){
    stats->_num_free_blocks--;
    stats->_num_free_bytes-=num_bytes;
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
    //res=(void*)((char*)res+1);
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
        *heap_bottom = MetaData(size,0,(void*)((char*)sbrk_res+sizeof(MetaData)),NULL);
        stats_allocate_block(size);
        return heap_bottom+1;
    }
    MetaData* itt= heap_bottom;
    while (itt){
        if (itt->isFree()&& itt->getSize()>=size){
            itt->setFree(0);
            stats_unfree_block(itt->getSize());
            return itt+1;
        }        
        else{
            if (itt->getNext()==NULL){// checked the last block, need to allocate a new one
                sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
                if (sbrk_res==NULL){
                    return NULL;
                }
                itt->setNext((MetaData*)sbrk_res);
                *(itt->getNext()) = MetaData(size,0,(void*)((char*)sbrk_res+sizeof(MetaData)),itt);
                stats_allocate_block(size);
                return itt->getNext()+1;
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
    if(p == NULL ){
        return;
    }
    MetaData* meta =(MetaData*)((char*)p-sizeof(MetaData));
    if (meta->isFree()){
        return;
    }
    stats_free_block(meta->getSize());
    meta->setFree(true);
}
void* srealloc(void* oldp, size_t size){
    if (oldp==NULL){
        return smalloc(size);
    }
    MetaData* meta =(MetaData*)((char*)oldp-sizeof(MetaData));
    if (meta->getSize()>= size && size > 0){
        return oldp;       
    }
    else{
        void* smalloc_res=smalloc(size);
        if (smalloc_res==NULL){
            return NULL;
        }
        std::memcpy(smalloc_res,(const void*)oldp,meta->getSize());
        sfree(oldp);
        return smalloc_res;
    }        
}

size_t _num_free_blocks(){
    return stats->_num_free_blocks;
}

size_t _num_free_bytes(){
    return stats->_num_free_bytes;
}

size_t _num_allocated_blocks(){
    return stats->_num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return stats->_num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return stats->_num_allocated_blocks * sizeof(MetaData);
}

size_t _size_meta_data(){
    return sizeof(MetaData);
}
