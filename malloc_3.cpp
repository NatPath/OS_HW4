#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <sys/mman.h>
#include "malloc_3.h"


#include <iostream>

#define HUNDERED_TWENTY_EIGHT 128
#define KILOB 1024
#define HUNDRED_MIL 100000000

 struct statistics{

    size_t _num_free_blocks;
    size_t _num_free_bytes;
    size_t  _num_allocated_blocks;
    size_t _num_allocated_bytes;

} stats_default{0,0,0,0};

typedef struct statistics* Stats;
Stats stats = &stats_default;



MetaData *heap_bottom=nullptr;
MetaData *wilderness=nullptr;

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

void init_meta_histogram(){
    for (int i=0 ;i <128 ;i++){
        //*meta_histogram[i]=MetaData(0,0,nullptr,nullptr);
        meta_histogram[i]=nullptr;
    }
}
//gets two nodes make a connection between them
//none of them should be null
void mutual_connect_frees(MetaData* prev,MetaData* next){
    prev->setNextFree(next);
    next->setPrevFree(prev);
}
//gets 3 nodes, inserts mid between prev and next
//none of them should be null
void insert_between_free_list(MetaData* prev, MetaData* mid ,MetaData* next){
    mutual_connect_frees(prev,mid);
    mutual_connect_frees(mid,next);
}
/**
 * gets a new entry and the smallest entery in the bin.
 * inserts the new entry and returns the new leftmost bin
 * should always set the start of the bin to the result of this function.
 * */
MetaData* insert_to_meta_bin(MetaData* new_entry, MetaData* bin_start){
    //handle corner case of new_entery being the smallest in the bin
    if (bin_start==nullptr){
        return new_entry;
    }
    if (bin_start->getSize()>=new_entry->getSize()){
        mutual_connect_frees(new_entry,bin_start);
        return new_entry;
    }
    //
    MetaData* prev= bin_start;
    MetaData* itt= bin_start->getNextFree();
    while(itt){
        if (itt->getSize()>=new_entry->getSize()){
            insert_between_free_list(prev,new_entry,itt);
        }
        else{
            prev=itt;
            itt=itt->getNextFree();
        }
    }
    mutual_connect_frees(prev,new_entry);
    return bin_start;
}
void insert_to_meta_histogram(MetaData* new_entry){
    size_t entery_size= new_entry->getSize();
    int index= (entery_size-1)/KILOB;
    if (index<=HUNDERED_TWENTY_EIGHT-1){
        meta_histogram[index]= insert_to_meta_bin(new_entry,meta_histogram[index]);
    }
    else{
        // entery_size >128KB
        //shouldn't get here
        std::cout<<"if you got here you fucked up" << std::endl;
        return;//???
    }
}
MetaData* search_bin_for_spot(int index, size_t entery_size){
    MetaData* itt= meta_histogram[index];
    while(itt){
        if (itt->getSize()>=entery_size){
            return itt;
        }
        itt = itt->getNextFree();
    }
    return nullptr;
}
/**
 * gets a block, searches the meta_histogram for a free block with fitting size 
 * if found, occupy that block ( while removing it from the free_list) 
 * if not , allocates a new block.
 * 
 * */
MetaData* search_histogram_for_spot(size_t entery_size){
    int index= (entery_size-1)/KILOB;
    MetaData* res=nullptr;
    if (index<=HUNDERED_TWENTY_EIGHT-1){
        res = search_bin_for_spot(index,entery_size);
        return res;
    }
    else{
        return nullptr; //???
    }
}
/**
 * does nothing but remove the node from the given bin
 * */
void remove_from_free_list(MetaData* to_unfree,int bin_index){
    MetaData* next = to_unfree->getNextFree();
    MetaData* prev = to_unfree->getPrevFree();   
    if (prev==nullptr ){
        meta_histogram[bin_index]=to_unfree->getNextFree();        
        if (next!=nullptr){
            next->setPrevFree(nullptr);            
        }
        return;
    }
    prev->setNextFree(next);
}
/**
 * gets a metadata of a block to unfree.
 * 1) set it to not free (duh)
 * 2) removes from free_histogram
 * 3) update free_stats
 * 
 * */
void unfree_block(MetaData* to_unfree,int bin_index){
    to_unfree->setFree(0);
    remove_from_free_list(to_unfree,bin_index);
    stats_unfree_block(to_unfree->getSize());    
}
void mutual_connect_corners(MetaData* a, MetaData* b){
    if (a==nullptr ){
        if(b){
            b->setPrev(nullptr);
        }
        return;
    }
    if (b==nullptr){
        if(a){
            a->setNext(nullptr);
        }
        return;
    }
    a->setNext(b);
    b->setPrev(a);

}

/**
 * 
 * 1) make two blocks out of one Block -> Block_A of size size,Block_B of size left
 * 2) remove block from free_list
 * 3) insert Block_B to free_list
 * 4) occupy Block_A 
 * 5) update stats
 * 6) update wilderness if needed
 * 7) return Block_A
 * 
 * */
MetaData* split_block(MetaData* block_to_split,int size,int index){
    remove_from_free_list(block_to_split,index);

    MetaData* block_a = block_to_split;
    MetaData* block_b = (MetaData*)((char*)block_to_split+sizeof(MetaData)+size);
    size_t size_left = block_to_split->getSize()-sizeof(MetaData)- size;
    //handeling pointers in the original block list
    *block_a = MetaData(size,0,block_a+1,block_to_split->getPrev(),0);
    *block_b = MetaData(size_left,1,block_b+1,block_a,0);
    block_a->setNext(block_b);
    block_b->setNext(block_to_split->getNext());
    //
    insert_to_meta_histogram(block_b);
    //update stats
    stats_unfree_block(size);
    if(block_to_split==wilderness){
        //update wilderness
        wilderness=block_b;
    }
    return block_a;
    
}

/**
 * search inside a bin for a block to fit in or to split
 * actually split it
 * update stats (inside split_block..)
 * 
 * */
MetaData* search_bin_and_split(size_t size,int index){
    MetaData* res=nullptr;
    MetaData* itt= meta_histogram[index];
    while(itt){
        if(itt->getSize() >= size ){
            if (itt->getSize() >= size+HUNDERED_TWENTY_EIGHT+sizeof(MetaData)){
                //split if splitable
                res = split_block(itt,size,index);
            }
            else{
                res = itt; 
            }
            break;
        }        
    }
    return res;
}
/**
 *  look for a bin to fit the new alloaction.
 *  find the smallest block in the heap.
 *  if it is splitable , split it.
 *  update stats(inside search_bin..)
 * 
 * */
MetaData* search_histogram_and_split(size_t size,int index){
    MetaData* res= nullptr;
    for (int i= index;i< HUNDERED_TWENTY_EIGHT;i++){
        res=search_bin_and_split(size,index);
        if (res){
            break;
        }
    }
    return res;
}
void remove_from_free_list_wo_index(MetaData* ){

}
/**
 * expands the wilderness to be able to fit the new block of size size
 * update stats
 * returns the wilderness (should stay the same if sbrk doesn't fail)
 * */
MetaData* expand_wilderness(size_t new_wilderness_size){
    size_t old_wilderness_size= wilderness->getSize();
    void* sbrk_res = sbrk_wrap(new_wilderness_size-old_wilderness_size);
    if (sbrk_res==nullptr){
        //indicating an error
        return nullptr;
    }
    int old_index = old_wilderness_size/KILOB;
    unfree_block(wilderness,old_index);    
    wilderness->setSize(new_wilderness_size);            
    stats->_num_allocated_bytes+= new_wilderness_size - old_wilderness_size;

    return wilderness; 
}
void* mmap_wrap(size_t size){

    void* res= mmap(NULL,size+sizeof(MetaData),PROT_EXEC|PROT_READ|PROT_WRITE,MAP_ANONYMOUS,-1,0);
    stats_allocate_block(size);
    return res;
}
/**
 * overall, makes the two adjecent free blocks into one big free block
 * 1) removes both the blocks from the histogram
 * 2) allocates a new block and inserts it into the histogram
 * 3) update statistics
 * 4) update wilderness
 * 
 * note: the case in which two blocks make a bigger than 128kb block will not be tested
 * */
MetaData* merge_two(MetaData* prev, MetaData* next){
    if (next == wilderness){
        wilderness=prev;
    }
    size_t prev_size= prev->getSize();
    size_t next_size= next->getSize();
    int prev_index = prev_size/KILOB;
    int next_index = next_size/KILOB;
    remove_from_free_list(prev, prev_index);            
    remove_from_free_list(next, next_index);            
    //handles pointers in original list
    *prev = MetaData(prev_size+next_size+sizeof(MetaData),1,prev+1,prev->getPrev(),0);
    mutual_connect_corners(prev->getPrev(),prev);
    mutual_connect_corners(prev,next->getNext());

    insert_to_meta_histogram(prev);
    stats_unfree_block(0);

    return prev;

}
/**
 * 
 * */
MetaData* merge_neighbours(MetaData* meta,int index){
    MetaData* next = meta->getNext();
    MetaData* prev = meta->getPrev();
    MetaData* res = meta;
    if (prev && prev->isFree()){
        res = merge_two(prev,res);
    }
    if (next && next->isFree()){
        res = merge_two(res,next);
    }
    return res; 

}
void* smalloc(size_t size){
    // start from the top
    int index= (size-1)/KILOB;
    MetaData* res;
    if (index<HUNDERED_TWENTY_EIGHT){
        //block is supposed to fit inside a bin
        void* sbrk_res;
        if (!heap_bottom){
            //first allocation on the heap
            init_meta_histogram();
            sbrk_res= sbrk_wrap(sizeof(MetaData)+size);
            if (sbrk_res==NULL){
                return NULL;
            }
            heap_bottom=(MetaData*)sbrk_res;
            *heap_bottom = MetaData(size,0,(void*)((char*)sbrk_res+sizeof(MetaData)),NULL,0);
            stats_allocate_block(size);
            wilderness= heap_bottom;
            return heap_bottom+1;
        }
        else{
            //not the first allocation
            res = search_histogram_and_split(size,index);
            if(res!=nullptr){
                //found a fitting block .if it was splitable we splat it
                return res+1;
            }
            //no fitting block or block to split
            if (wilderness->isFree()){
                res = expand_wilderness(size);
                if (res!=nullptr){
                    return res+1;
                }
                else{
                    return nullptr;
                }
            }
            //wilderness is not free, we must sbrk() like animals
            sbrk_res = sbrk_wrap(size+sizeof(MetaData));
            if (sbrk_res==NULL){
                return NULL;
            }
            stats_allocate_block(size);
            return sbrk_res+sizeof(MetaData);
        }
    }
    else{
        //block too big for a bin
        mmap_wrap(size);
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
    int old_index = meta->getSize()/KILOB;
    if (old_index<HUNDERED_TWENTY_EIGHT){
        merge_neighbours(meta,old_index);
    }
    else{
        int munnmap_res = munmap((void*)meta,sizeof(MetaData)+meta->getSize());
        if (munnmap_res==-1){
            std::cout<< "failed munmmap" <<std::endl;
        }
        // i think we don't need to do anything about the statistics in this case
    }
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
