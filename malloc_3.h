#ifndef _MALLOC3_H_ 
#define _MALLOC3_H_
#include <unistd.h>
#include <iostream>

#define Println(expression) do {\
    std::cout<<expression<<std::endl;\
}\
while(0)\

void printStats();
class MetaData{
    size_t  _size;
    bool _is_free;
    void* _data_block;
    MetaData* _next;
    MetaData* _prev;
    MetaData* _next_free;
    MetaData* _prev_free;
    MetaData* _next_mmap;
    MetaData* _prev_mmap;
    bool _is_mmaped;

    public:
    MetaData(size_t size ,bool is_free,void* data_block, MetaData* last_metadata,bool is_mmaped):_size(size),_is_free(is_free),_data_block(data_block),_prev(last_metadata),_next(nullptr),_is_mmaped(is_mmaped){
        _next_free=nullptr;
        _prev_free=nullptr;
        _next_mmap=nullptr;
        _prev_mmap=nullptr;
    }
    size_t getSize(){
        return _size;
    }
    void setSize(size_t size){
        _size = size;
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
    MetaData* getNextFree(){
        return _next_free;
    }
    void setNextFree(MetaData* next_free){
        _next_free=next_free;
    }
    MetaData* getPrevFree(){
        return _prev_free;
    }
    void setPrevFree(MetaData* prev_free){
        _prev_free=prev_free;
    }

    void insertFreeEntry(MetaData* new_entry){
        MetaData* itt= this;
        while(itt->getNext()){
                        
        }
        itt->setNextFree(new_entry);
    }
    void insertFreeEntryBetween(MetaData* new_entry,MetaData* prev, MetaData* next){
        if (next==nullptr){}
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

#endif