#pragma once
#include <cstdint>
#include <vector>

namespace attendance::visibility {
struct Pass {unsigned calls=0,verified=0,changed=0;};
struct Result {unsigned controllers=0,calls=0,verified=0,changed=0;};
// A controller resolves scene crowds. Its layout is not a crowd renderer's.
// Only the resulting native visibility callbacks confirm a renderer update.
template<class Validate,class Update>
Result dispatch(const std::vector<uintptr_t>& controllers,bool captureOnly,bool captureBaseline,Validate validate,Update update){
    Result result;
    if(captureOnly||captureBaseline){
        for(auto controller:controllers){
            if(!validate(controller))continue;
            Pass pass=update(controller,true);
            if(captureOnly){++result.controllers;result.calls+=pass.calls;result.verified+=pass.verified;}
        }
    }
    if(!captureOnly){
        for(auto controller:controllers){
            if(!validate(controller))continue;
            Pass pass=update(controller,false);
            ++result.controllers;result.calls+=pass.calls;result.verified+=pass.verified;result.changed+=pass.changed;
        }
    }
    return result;
}
}
