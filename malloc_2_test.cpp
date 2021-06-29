//ofir and einav wohoo
#include <iostream>
#include "malloc_2.h"
#include <cmath>
#define NUM_OF_ARRAYS 11
/*
 *
 *
 * compile  g++ -o test malloc2_test.cpp;
 * run: ./test
 *
 *
funcs to check:
	smalloc(size_t size)
	scalloc(size_t num, size_t size)
    sfree(void* p)
	srealloc(void* oldp, size_t size)

debug funcs:
	_num_free_blocks()
	_num_free_bytes()
	_num_allocated_blocks()
	_num_allocated_bytes()
	_num_meta_data_bytes()
	_size_meta_data()

*/

bool debugSuccess(size_t numFree, size_t numAlloc, size_t numFreeBytes, size_t numAllocBytes){
    bool flag= true;
    size_t allocated_blocks = _num_allocated_blocks();
    size_t free_blocks = _num_free_blocks();
    size_t allocated_bytes = _num_allocated_bytes();
    size_t  free_bytes = _num_free_bytes();
    size_t metadata_bytes = _num_meta_data_bytes();

    flag = flag &&  free_blocks == numFree;
    flag = flag &&  allocated_blocks == numAlloc;
    flag = flag &&  allocated_bytes == numAllocBytes;
    flag = flag &&  free_bytes == numFreeBytes;
    flag = flag &&  metadata_bytes == numAlloc * _size_meta_data();
    return flag;
}

int main(){
    bool passed_tot = true;
    bool passed_loc;
    char* all_arrays[NUM_OF_ARRAYS];
    int arr_sizes[NUM_OF_ARRAYS];
    arr_sizes[0]= 0;
    for(int i=1 ; i<NUM_OF_ARRAYS; i++){
        arr_sizes[i]= (int)pow(10, i-1);
    }
    size_t totalsize=0;
    for(int i=1; i<NUM_OF_ARRAYS-1; i++){
        totalsize = totalsize+arr_sizes[i];
    }
    std::cout << "\n \n basic testing!!!!!! \n\n" << std::endl;

    std::cout << "\n malloc test \n" << std::endl;
    for(int i=0; i<NUM_OF_ARRAYS; i++){
        all_arrays[i] = (char*) smalloc(arr_sizes[i]);
    }

    passed_loc = all_arrays[0] == NULL && all_arrays[NUM_OF_ARRAYS - 1] == NULL;
    if(!passed_loc){
        std::cout << "\nerror smalloc parameters1 \n" << std::endl;
    }
    passed_tot = passed_tot && passed_loc;

    passed_loc = debugSuccess(0, NUM_OF_ARRAYS - 2, 0, totalsize);
    if(!passed_loc){
        std::cout << "\nerror smalloc parameters2 \n" << std::endl;
    };
    passed_tot = passed_tot && passed_loc;

    std::cout << "\nfree test \n" << std::endl;
    sfree(all_arrays[NUM_OF_ARRAYS-3]);
    all_arrays[NUM_OF_ARRAYS-3]=NULL;

    passed_loc= debugSuccess(1, NUM_OF_ARRAYS - 2, arr_sizes[NUM_OF_ARRAYS - 3], totalsize);
    if(!passed_loc){
        std::cout << "\nerror free \n" << std::endl;
    };
    passed_tot = passed_tot && passed_loc;

    std::cout << "\n calloc test \n" << std::endl;
    all_arrays[NUM_OF_ARRAYS-3] = (char*)scalloc(sizeof(char), arr_sizes[NUM_OF_ARRAYS-3] + 5);

    passed_loc= debugSuccess(1, NUM_OF_ARRAYS - 1, arr_sizes[NUM_OF_ARRAYS - 3], totalsize + arr_sizes[NUM_OF_ARRAYS-3] + 5);
    if(!passed_loc){
        std::cout << "\nerror calloc or malloc set up \n" << std::endl;
    };
    passed_tot = passed_tot && passed_loc;

    passed_loc= true;
    for(int i=0; i< arr_sizes[NUM_OF_ARRAYS-3] + 5; i++ ){
        passed_loc = passed_loc && all_arrays[NUM_OF_ARRAYS-3][i] == 0;
    }

    if(!passed_loc){
        std::cout << "\nerror int calloc init  \n" << std::endl;
    };
    passed_tot = passed_tot && passed_loc;



    std::cout << "\n realloc1 test \n" << std::endl;
    all_arrays[0]= (char*)srealloc((void*) all_arrays[NUM_OF_ARRAYS-3], arr_sizes[NUM_OF_ARRAYS-3]);
    if(all_arrays[0]!=NULL){
        all_arrays[NUM_OF_ARRAYS-3]= NULL;
        passed_loc= debugSuccess(1, NUM_OF_ARRAYS - 1, arr_sizes[NUM_OF_ARRAYS - 3], totalsize + arr_sizes[NUM_OF_ARRAYS-3] + 5);
        if(!passed_loc){
            std::cout << "\nerror realloc1 set up \n" << std::endl;
        };
        passed_tot = passed_tot && passed_loc;

        passed_loc= true;
        for(int i=0 ; i<arr_sizes[NUM_OF_ARRAYS-3]; i++){
            passed_loc= passed_loc && all_arrays[0][i] == 0;
        }
        if(!passed_loc){
            std::cout << "\nerror realloc1 copy \n" << std::endl;
        };
        passed_tot = passed_tot && passed_loc;
    }

    std::cout << "\n realloc2 test \n" << std::endl;
    for(int i=0 ; i<arr_sizes[NUM_OF_ARRAYS-4]; i++){
        all_arrays[NUM_OF_ARRAYS-4][i] = 0;
    }
    char* temp = (char*) srealloc(all_arrays[NUM_OF_ARRAYS-4], arr_sizes[NUM_OF_ARRAYS-4]+1);
    if(temp != NULL){
        all_arrays[NUM_OF_ARRAYS-4]= temp;
        passed_loc= debugSuccess(1, NUM_OF_ARRAYS - 1, arr_sizes[NUM_OF_ARRAYS - 4], totalsize + arr_sizes[NUM_OF_ARRAYS-3] + 5);
        if(!passed_loc){
            std::cout << "\nerror realloc2 set up \n" << std::endl;
        };
        passed_tot = passed_tot && passed_loc;

        passed_loc= true;
        for(int i=0 ; i<arr_sizes[NUM_OF_ARRAYS-4]; i++){
            passed_loc= passed_loc && all_arrays[NUM_OF_ARRAYS-4][i] == 0;
        }
        if(!passed_loc){
            std::cout << "\nerror realloc2 copy \n" << std::endl;
        };
        passed_tot = passed_tot && passed_loc;
    }

    std::cout << "\nfree all \n" << std::endl;

    for(int i =0; i<NUM_OF_ARRAYS; i++){
        if(all_arrays[i]){
            sfree((void*)all_arrays[i]);
        }
        all_arrays[i]=NULL;
    }

    passed_loc= debugSuccess(NUM_OF_ARRAYS-1, NUM_OF_ARRAYS - 1, totalsize + arr_sizes[NUM_OF_ARRAYS-3] + 5, totalsize + arr_sizes[NUM_OF_ARRAYS-3] + 5);
    if(!passed_loc){
        std::cout << "\nerror free all \n" << std::endl;
    };
    passed_tot = passed_tot && passed_loc;

    if(passed_tot){
        std::cout << "\n \n PASS!!!!!! KOL-HAKAVOD!!!!! \n\n" << std::endl;
    }
    else{
        std::cout << "\n \n FAILED!!!!!!\n\n" << std::endl;
    }

    return 0;
}