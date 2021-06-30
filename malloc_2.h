#ifndef _MALLOC2_H_ 
#define _MALLOC2_H_


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

#endif