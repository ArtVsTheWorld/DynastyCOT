#include "../Source/NativeRead.h"
#include <cassert>
#include <cstring>
#include <iostream>
int main(){
    auto partial=[](uintptr_t,void* out,size_t n){std::memset(out,0xff,n/2);return false;};
    assert(attendance::nativeValue<uintptr_t>(123,partial)==0);
    assert(attendance::nativeValue<unsigned>(123,partial)==0);
    assert(attendance::nativeValue<float>(123,partial)==0);
    auto complete=[](uintptr_t,void* out,size_t n){assert(n==sizeof(unsigned));unsigned v=42;std::memcpy(out,&v,n);return true;};
    assert(attendance::nativeValue<unsigned>(123,complete)==42);
    std::cout<<"PASS: failed reads discard partially modified pointers, flags and floating values; successful reads retain exact values.\n";
}
