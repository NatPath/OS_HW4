//ofir and einav wohoo
#include "malloc_4.cpp"
#include <assert.h>
#include <iostream>
#define ARR_PTR_SIZE 15


int main(){
    void*  arr_ptr[ARR_PTR_SIZE];
    std::cout << "\ntesting only malloc  \n" << std::endl;
    for(int i=0 ; i<ARR_PTR_SIZE; i++){
        arr_ptr[i] = smalloc(i*500);
        assert(((long int)arr_ptr[i])%8 == 0);
    }

    for(int i=0 ; i<ARR_PTR_SIZE; i+=2){
        sfree(arr_ptr[i]);
        arr_ptr[i]= NULL;
    }

    std::cout << "\ntesting malloc after free  \n" << std::endl;
    
    for(int i=ARR_PTR_SIZE-1 ; i>10; i--){
        arr_ptr[i] = smalloc(i*501);
        assert(((long int)arr_ptr[i])%8 == 0);
    }

    std::cout << "\ntesting realloc after free  \n" << std::endl;
    for(int i=0 ; i<10; i++){
        arr_ptr[i] = srealloc(arr_ptr[i], (i+2)*500);
        assert(((long int)arr_ptr[i])%8 == 0);
    }

    for(int i=0 ; i<ARR_PTR_SIZE; i++){
        if(arr_ptr[i]!= NULL){
            sfree(arr_ptr[i]);
            arr_ptr[i]=NULL;
        }
    }
    std::cout << "\ntesting calloc after free  \n" << std::endl;
    for(int i=0 ; i<ARR_PTR_SIZE; i++){
        arr_ptr[i] = scalloc(i*500, sizeof(char));
        assert(((long int)arr_ptr[i])%8 == 0);
    }

    for(int i=0 ; i<ARR_PTR_SIZE; i++){
        char* temp = (char*) arr_ptr[i];
        for(int j=0; j<i*500; j++){
            assert(temp[j]==0);
        }
    }

    for(int i=0 ; i<ARR_PTR_SIZE; i++){
        if(arr_ptr[i]!= NULL){
            sfree(arr_ptr[i]);
            arr_ptr[i]=NULL;
        }
    }

    std::cout << "\nSUCCESS!!!!!! you did it! you are the best of the best \n" << std::endl;
    return 0;
}
