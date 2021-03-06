#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <sys/mman.h>



/*
// added from malloc_3.h
#define Println(expression) do {\
    std::cout<<expression<<std::endl;\
}\
while(0)\
*/

//void printStats();
class MetaData{

    public:
    size_t  _size;
    bool _is_free;
    void* _data_block;
    MetaData* _next;
    MetaData* _prev;
    MetaData* _next_free;
    MetaData* _prev_free;
    bool _is_mmaped;
    MetaData(size_t size ,bool is_free,void* data_block, MetaData* last_metadata,bool is_mmaped):_size(size),_is_free(is_free),_data_block(data_block),_prev(last_metadata),_next(nullptr),_is_mmaped(is_mmaped){
        _next_free=nullptr;
        _prev_free=nullptr;
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
Stats our_stats = &stats_default;
MetaData* meta_histogram[128];



MetaData *heap_bottom=nullptr;
MetaData *wilderness=nullptr;

size_t _num_free_blocks(){
    return our_stats->_num_free_blocks;
}

size_t _num_free_bytes(){
    return our_stats->_num_free_bytes;
}

size_t _num_allocated_blocks(){
    return our_stats->_num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return our_stats->_num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return our_stats->_num_allocated_blocks * sizeof(MetaData);
}

size_t _size_meta_data(){
    return sizeof(MetaData);
}

/*
void printStats(){
    Println("Stats: ");
    Println("free blocks: " << _num_free_blocks());
    Println("free bytes: "<< _num_free_bytes());
    Println("allocated blocks: "<< _num_allocated_blocks());
    Println("allocated bytes: "<< _num_free_bytes());
}
*/

void stats_allocate_block(size_t num_bytes){
     our_stats->_num_allocated_blocks++;
     our_stats->_num_allocated_bytes+=num_bytes;
}

void stats_free_block(size_t num_bytes){
    our_stats->_num_free_blocks++;
    our_stats->_num_free_bytes+=num_bytes;
}

void stats_unfree_block(size_t num_bytes){
    our_stats->_num_free_blocks--;
    our_stats->_num_free_bytes-=num_bytes;
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
    MetaData* prev = bin_start;
    MetaData* itt = bin_start->getNextFree();
    while(itt){
        if (itt->getSize()>=new_entry->getSize()){
            insert_between_free_list(prev,new_entry,itt);
            break;
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
    if (index<HUNDERED_TWENTY_EIGHT){
        meta_histogram[index]= insert_to_meta_bin(new_entry,meta_histogram[index]);
    }
    else{
        // entery_size >128KB
        //shouldn't get here
      //  std::cout<<"if you got here you fucked up" << std::endl;
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
    if (index<HUNDERED_TWENTY_EIGHT){
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
    if (next!=nullptr){
        next->setPrevFree(prev);
    }
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
 * 5) update stats - unfree_block(size) , _num_allocated_blocks++
 * 6) update wilderness if needed
 * 7) return Block_A
 * 
 * */
MetaData* split_block(MetaData* block_to_split,int size,int index){
    remove_from_free_list(block_to_split,index);
    MetaData* block_a = block_to_split;
    MetaData* block_b = (MetaData*)((char*)block_to_split+sizeof(MetaData)+size);
    MetaData* prev = block_to_split->getPrev();
    MetaData* next = block_to_split->getNext();

    size_t size_left = block_to_split->getSize()-sizeof(MetaData)- size;
    //handeling pointers in the original block list
    *block_a = MetaData(size,0,block_a+1,block_to_split->getPrev(),0);
    *block_b = MetaData(size_left,1,block_b+1,block_a,0);
    block_a->setNext(block_b);
    block_b->setNext(block_to_split->getNext());
    mutual_connect_corners(prev,block_a);
    mutual_connect_corners(block_b,next);

    //
    insert_to_meta_histogram(block_b);
    //update stats
    our_stats->_num_free_bytes-=size+sizeof(MetaData);
    //stats_allocate_block(-sizeof(MetaData)); // equivalent to stats->_num_allocated_blocks++
    our_stats->_num_allocated_bytes-=sizeof(MetaData);
    our_stats->_num_allocated_blocks++;
    if(block_to_split==wilderness){
        //update wilderness
        wilderness=block_b;
    }
    return block_a;
    
}

/*
void printMetaData(MetaData* meta){
    Println("---Printing Block---");
    Println("size: " <<meta->getSize());
    Println("is free?: " <<meta->isFree());
    Println("data location : " <<meta->getDataBlock());
    Println("next is :" << meta->getNext());
    Println("prev is :" << meta->getPrev());
}
*/

/**
 * search inside a bin for a block to fit in or to split
 * actually split it
 * update stats 
 * update wilderness(?)
 * 
 * */
MetaData* search_bin_and_split(size_t size,int index){
    //Println("entered search_bin_and_split");
    /*
    if (index ==2 ) {
        Println(" got to index 2");
        printMetaData(meta_histogram[2]);
    }
    */
    MetaData* res=nullptr;
    MetaData* itt= meta_histogram[index];
    while(itt!=nullptr){
        if(itt->getSize() >= size ){
            if (itt->getSize() >= size+HUNDERED_TWENTY_EIGHT+sizeof(MetaData)){
                //split if splitable
                res = split_block(itt,size,index);
            }
            else{
                //unsplitable block
                res = itt; 
                res->setFree(0);
                remove_from_free_list(res,index);
                stats_unfree_block(itt->getSize());
            }
            break;
        }        
        itt= itt->getNextFree();
    }
    return res;
}
/**
 *  look for a bin to fit the new alloaction.
 *  find the smallest block in the heap.
 *  if it is splitable , split it.
 *  update stats(inside search_bin..)
 * update wilderness(?)
 * 
 * */
MetaData* search_histogram_and_split(size_t size,int index){
    MetaData* res= nullptr;
    //print_free_histogram();
    for (int i= index;i< HUNDERED_TWENTY_EIGHT;i++){
        //Println("bin of index : " << i );
        res=search_bin_and_split(size,i);
        if (res){
            break;
        }
    }
    return res;
}

/**
 * expands the wilderness to be able to fit the new block of size size
 * update stats - only if the wilderness was free!
 * returns the wilderness (should stay the same if sbrk doesn't fail)
 * */
MetaData* expand_wilderness(size_t new_wilderness_size){
    size_t old_wilderness_size= wilderness->getSize();
    //void* sbrk_res = sbrk_wrap(new_wilderness_size-old_wilderness_size);
    void* sbrk_res = sbrk(new_wilderness_size-old_wilderness_size);
    if (sbrk_res==(void*)(-1)){
        //indicating an error
        return nullptr;
    }
    int old_index = (old_wilderness_size-1)/KILOB;
    if (wilderness->isFree()){
        unfree_block(wilderness,old_index);    
        our_stats->_num_allocated_bytes+= new_wilderness_size - old_wilderness_size;
    }
    wilderness->setSize(new_wilderness_size);            

    return wilderness; 
}
void* mmap_wrap(size_t size){

    void* res= mmap(NULL,size+sizeof(MetaData),PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
    if (res== (void*)(-1)){
        return nullptr;
    }
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
    int prev_index = (prev_size-1)/KILOB;
    int next_index = (next_size-1)/KILOB;
    if (prev->isFree()){
        remove_from_free_list(prev, prev_index);            
    }
    if (next->isFree()){
        remove_from_free_list(next, next_index);            
    }
    //handles pointers in original list
    *prev = MetaData(prev_size+next_size+sizeof(MetaData),1,prev+1,prev->getPrev(),0);
    mutual_connect_corners(prev->getPrev(),prev);
    mutual_connect_corners(prev,next->getNext());

    insert_to_meta_histogram(prev);
    stats_unfree_block(0);
    our_stats->_num_allocated_blocks--;
    our_stats->_num_allocated_bytes+=sizeof(MetaData);
    our_stats->_num_free_bytes+=sizeof(MetaData);

    return prev;

}
/**
 *  merge left to meta
 *  merge meta to right
 *  update stats
 * Note:
 *  usually merge_two handles the wilderness and update stats
 *  if did not merge updates the stats and wilderness manually 
 * */
MetaData* merge_neighbours(MetaData* meta,int index){
    
    size_t meta_size= meta->getSize();
    MetaData* next = meta->getNext();
    MetaData* prev = meta->getPrev();
    MetaData* res = meta;
    bool did_merge= false;
    if (prev && prev->isFree()){
        did_merge = true;
        res = merge_two(prev,res);
    }
    if (next && next->isFree()){
        did_merge = true;
        res = merge_two(res,next);
    }
    if (!did_merge){
        meta->setFree(1);
        insert_to_meta_histogram(meta);
    }
    return res; 

}
void* smalloc(size_t size){
    // start from the top
    if (size==0 || size > HUNDRED_MIL){
        return nullptr;
    }
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
            heap_bottom->setNext(nullptr);
            stats_allocate_block(size);
            wilderness= heap_bottom;
            return heap_bottom+1;
        }
        else{
            //not the first allocation
            int allocated_blocks_before= _num_allocated_blocks();
            res = search_histogram_and_split(size,index);
            int allocated_blocks_after = _num_allocated_blocks();
            if(res!=nullptr){
                /*
                //found a fitting block .if it was splitable we splat it
                if (allocated_blocks_after == allocated_blocks_before){
                    // we haven't split anything, so we need to update the stats
                    stats_allocate_block()
                }
                */
                return res+1;
            }
            //no fitting block or block to split
            if (wilderness->isFree()){
                //Println("--expanded wilderness--");
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
            //Println("sbrk like animals");
            res = (MetaData*)sbrk_res;
            *res = MetaData(size,0,res+1,wilderness,0);
            wilderness->setNext(res);
            wilderness=res;
            stats_allocate_block(size);
            return res+1;
        }
    }
    else{
        //block too big for a bin
        void* mmap_res = mmap_wrap(size);
        if (mmap_res==nullptr){
            return nullptr;
        }

        res = (MetaData*)mmap_res;
        *res = MetaData(size,0,res+1,nullptr,1);
        stats_allocate_block(size);
        return res+1;

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
void sfree_wo_merge(void* p){
    if(p == NULL ){
        return;
    }
    MetaData* meta =(MetaData*)((char*)p-sizeof(MetaData));
    //
    if (meta->isFree()){
        return;
    }
    int old_size = meta->getSize();
    int old_index = (old_size-1)/KILOB;
    if (old_index<HUNDERED_TWENTY_EIGHT){
        stats_free_block(old_size);
        //merge_neighbours(meta,old_index);
        // Println("just freed a block of size: " << old_size);
    }
    else{
        int munnmap_res = munmap((void*)meta,sizeof(MetaData)+meta->getSize());
        our_stats->_num_allocated_bytes-=old_size;
        our_stats->_num_allocated_blocks--;
    }
}
void sfree(void* p){
    if(p == NULL ){
        return;
    }
    MetaData* meta =(MetaData*)((char*)p-sizeof(MetaData));
    //
    if (meta->isFree()){
        return;
    }
    int old_size = meta->getSize();
    int old_index = (old_size-1)/KILOB;
    if (old_index<HUNDERED_TWENTY_EIGHT){
        stats_free_block(old_size);
        merge_neighbours(meta,old_index);
        // Println("just freed a block of size: " << old_size);
    }
    else{
        int munnmap_res = munmap((void*)meta,sizeof(MetaData)+meta->getSize());
        our_stats->_num_allocated_bytes-=old_size;
        our_stats->_num_allocated_blocks--;
    }
}
/**
 * gets a free block and a size_to_fit .
 * merge the blocks in the neighbourhood just enough to fit the required size_to_fit
 * returns a pointer to the new contingous free block 
 * if wasn't able to fit, return nullptr
 * 
 * NOTE: after the merge the block will be regarded as free
 * 
 * */
MetaData* try_to_merge(MetaData* curr, size_t size_to_fit){
    size_t curr_size= curr->getSize();
    MetaData* prev = curr->getPrev();
    MetaData* next = curr->getNext();
    MetaData* res = nullptr;
    bool prev_and_meta_big_enough = false;
    if (prev && prev->isFree() && prev->getSize() + curr_size + sizeof(MetaData) >= size_to_fit){
        prev_and_meta_big_enough = true;
    }
    bool next_and_meta_big_enough = false;
    if( next && next->isFree() && next->getSize() + curr_size + sizeof(MetaData) >= size_to_fit){
        next_and_meta_big_enough = true;
    }
    if (prev_and_meta_big_enough){
        //merging with left
        res = merge_two(prev,curr) ;       
        return res;
    }
    if (next_and_meta_big_enough){
        //merging with right
        res = merge_two(curr,next);
        return res;
    }
    bool together_big_enough = false;
    if (next && next->isFree() && prev && prev->isFree() &&  (prev->getSize() + curr_size + next->getSize()+ 2*sizeof(MetaData) >= size_to_fit ) ){
        together_big_enough =true;
    }
    if ( together_big_enough){
        res = merge_neighbours(curr,size_to_fit);
        return res;
    }
    return res;

    
}
MetaData* copy_data_wrap(MetaData* dest, void* source, size_t size_of_data){
    void* dest_data = dest->getDataBlock();
    void* res_data =memmove(dest_data,source,size_of_data);
    if (res_data==nullptr){
        return nullptr;
    }
    else{
        return dest;        
    }
}

void* srealloc(void* oldp, size_t size){
    if (oldp==NULL){
        return smalloc(size);
    }
    if (size==0 || size>HUNDRED_MIL){
        return nullptr;
    }
    MetaData* meta =(MetaData*)((char*)oldp-sizeof(MetaData));
    int old_size= meta->getSize();
    int old_index = (old_size-1)/KILOB;
    void* smalloc_res=nullptr;
    MetaData* res=nullptr;
    size_t min_size = old_size<size?old_size:size;
    if (old_index<HUNDERED_TWENTY_EIGHT){
        //heap stuff
        MetaData* prev = meta->getPrev();
        MetaData* next = meta->getNext();

        //(1)
        if (meta->getSize()>= size && size > 0){
            //try to reuse (a)
            res=meta;
            insert_to_meta_histogram(res);
            stats_free_block(res->getSize());
        }
        else{
            // b c d
            res = try_to_merge(meta, size);
            
            if (res==nullptr){
                // cases a-d failed
                if (meta==wilderness){
                    //sfree(oldp);
                    /*
                    insert_to_meta_histogram(res);
                    stats_free_block(res->getSize());
                    */

                    res =expand_wilderness(size);
                    /*
                    if (res!=wilderness){
                        Println("something went wrong with wilderness block");
                    }
                    */
                    our_stats->_num_allocated_bytes+=size-old_size;
                    return res->getDataBlock();
                }
                else{
                    smalloc_res = smalloc(size);
                    memmove(smalloc_res,oldp,min_size);
                    sfree(oldp);
                    return smalloc_res;
                }
            }
            stats_free_block(old_size);

        }
        // (2)
        int new_index;
        //one of a-d worked
        new_index = (res->getSize()-1)/KILOB;
        if (res->getSize() >= size+HUNDERED_TWENTY_EIGHT+sizeof(MetaData)){
            //split if splitable
            //sfree(res->getDataBlock());
            res = split_block(res,size, new_index);
            memmove(res->getDataBlock(),oldp,min_size);
            return res+1;
        }
        new_index = (res->getSize()-1)/KILOB;
        unfree_block(res,new_index);

        /*
        remove_from_free_list(res,new_index);        
        stats_unfree_block(size-res->getSize());
        //unfree_block(res,new_index);
        */
        memmove(res->getDataBlock(),oldp,min_size);
        return res+1;
    }
    else{
        smalloc_res = smalloc(size);
        res = (MetaData*)((char*)smalloc_res-sizeof(MetaData));
        res = copy_data_wrap(res,oldp,min_size);
        sfree(oldp);
        if (res== nullptr){
            return nullptr;
        }
        return res+1;
    }
}

