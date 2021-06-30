#include <iostream>
#include "malloc_2.h"

#define Println(expression) do {\
    std::cout<<expression<<std::endl;\
}\
while(0)\

void printStats(){
    Println("Stats: ");
    Println("free blocks: " << _num_free_blocks());
    Println("free bytes: "<< _num_free_bytes());
    Println("allocated blocks: "<< _num_allocated_blocks());
    Println("allocated bytes: "<< _num_free_bytes());

}

class Dummy{
    int _a;
    int _b;
    public:
    Dummy(int a,int b):_a(a),_b(b){};
    int getA(){return _a;};
    int getB(){return _b;};
};

int main(){

    void* p = smalloc(sizeof(int)*2);
    Println("p is in : " << p);
    //printStats();
    *(Dummy*)p= Dummy(1123123,-232);
    Println("Data inside dummy:");
    Println(((Dummy*)p)->getA());
    Println(((Dummy*)p)->getB());
    MetaData* meta_p= (MetaData*)((char*)p-sizeof(MetaData));
    Println("meta_p is in :" << meta_p);
    Println("size of meta_data is: "<< _size_meta_data());
    Println("size of data pointed by p : " <<meta_p->getSize());
    Println("is free of data pointed by p : " <<meta_p->isFree());
    Println("data block is in: " << meta_p->getDataBlock());
    


    sfree(p);
    //printStats();

}