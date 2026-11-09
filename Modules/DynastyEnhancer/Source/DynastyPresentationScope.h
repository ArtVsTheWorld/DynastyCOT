#pragma once
#include "EnhancementOptions.h"

namespace enhancements {
inline bool presentationScope(int mode,int session,bool ownedCareer,bool active,uint32_t league,uint32_t selected){
    return (mode==14||mode==-1)&&session==3&&ownedCareer&&active&&league&&league!=UINT32_MAX&&league==selected;
}
#if defined(_WIN32)
inline uintptr_t presentationImage=0;
inline bool nativePresentationScope(){
    // The loaded Dynasty frontend can use -1; that value alone never admits
    // presentation. A live owned career and matching profile are mandatory.
    // Guarded in-process scalar reads, not RPM, file polling, locking or a
    // complete career/database ownership walk on every rating label.
    __try{
        const auto image=presentationImage;if(!image)return false;
        const auto mode=*reinterpret_cast<volatile int*>(image+0xd9eb808);
        const auto session=*reinterpret_cast<volatile int*>(image+0xf3fddbc);
        if((mode!=14&&mode!=-1)||session!=3)return false;
        const auto career=*reinterpret_cast<volatile uintptr_t*>(image+0xfa519b8);
        if(!career)return false;
        return presentationScope(mode,session,*reinterpret_cast<volatile uintptr_t*>(career)==image+0xbaa8998,
            *reinterpret_cast<volatile unsigned char*>(career+0x7b1)==1,
            *reinterpret_cast<volatile uint32_t*>(image+0xf55c998),currentOptionsLeague.load(std::memory_order_relaxed));
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline void requireDynastyPresentation(uintptr_t image){
    presentationImage=image;presentationGuard.store(nativePresentationScope,std::memory_order_release);
}
#endif
}
