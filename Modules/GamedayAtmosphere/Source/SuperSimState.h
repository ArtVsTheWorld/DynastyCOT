#pragma once
#include "GameBindings.h"
namespace attendance::simulation {
// Mode 1 alone includes visible Slow Sim. Normal/Fast can hold either None
// or PlayCall between simulated plays. PlayCall is already a verified safe
// crowd-change phase; live pre-snap/snap phases remain excluded in every mode.
template<class Read>bool offscreen(Read read,uintptr_t image,int phase){
    if(phase!=0&&phase!=5)return false;
    uintptr_t controller=0,vtable=0;int current=-1,next=-1;
    return read(image+bindings::SuperSimController,&controller,sizeof(controller))&&controller>=0x10000&&
        read(controller,&vtable,sizeof(vtable))&&vtable==image+bindings::SuperSimVtable&&
        read(controller+8,&current,sizeof(current))&&read(controller+12,&next,sizeof(next))&&current==1&&next==1;
}
}
