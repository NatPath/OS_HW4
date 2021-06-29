//ofir and einav wohoo
#include "malloc_1.cpp"
#include <iostream>

int main(){
    void* p1 = smalloc(0);
    if(p1 != NULL){
        std::cout << "error: malloc 0 succeed!" << std::endl;
    }
    void* p2 = smalloc(100000001);
    if(p2 != NULL){
        std::cout << "error: malloc 100000001 succeed!" << std::endl;
    }
    void* p3 = smalloc(100000000);
    if(p3 == NULL){
        std::cout << "error: malloc 100000000 did not succeed!" << std::endl;
    }
    void* p4 = smalloc(17);
    if(p4 == NULL){
        std::cout << "error: malloc 17 did not succeed!" << std::endl;
    }

    std::cout << "malloc_1 is great and you are amazing!" << std::endl;
    return 0;
}
