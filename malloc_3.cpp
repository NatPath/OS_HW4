#include <unistd.h>
#include <stdlib.h>
#include <cstring>


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
    MetaData* _next_free;
    MetaData* _prev_free;
    public:
    MetaData(size_t size ,bool is_free, MetaData* last_metadata):_size(size),_is_free(is_free),_prev(last_metadata),_next(nullptr),_prev_free(nullptr),_next_free(nullptr){
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
    MetaData* getNextFree(){
        return _next_free;
    }
    void setNextFree(MetaData* nextFree){
        _next_free=nextFree;
    }
    MetaData* getPrevFree(){
        return _prev_free;
    }
    void setPrevFree(MetaData* prev_free){
        _prev_free=prev_free;
    }


};

void stats_allocate_block(size_t num_bytes){
     stats._num_allocated_blocks++;
     stats._num_allocated_bytes+=num_bytes;
}

MetaData *heap_bottom=nullptr;
statistics stats = stats_default;
MetaData *histogram[128];

//increses the heap size by size (allocates a block). returns the address of the new allocated block
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
        *heap_bottom = MetaData(size,0,NULL);
        stats_allocate_block(size);
        for (int i=0;i<128;i++){// initilize free block histogram list
            *histogram[i]=MetaData(0,1,nullptr);
        }
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

    MetaData* meta =(MetaData*)(p-sizeof(MetaData));
    stats._num_free_blocks++;
    stats._num_free_bytes+=meta->getSize();
    meta->setFree(true);

    //update free list
    insertFreeBlock(meta);

    
    
    
}
void* srealloc(void* oldp, size_t size){
    MetaData* meta =(MetaData*)(oldp-sizeof(MetaData));
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