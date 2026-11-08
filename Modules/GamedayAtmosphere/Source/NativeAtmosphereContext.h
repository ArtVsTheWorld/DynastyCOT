#pragma once
#include "SwellAudioRuntime.h"

namespace attendance::nativecontext {
using sustained::Address;
using sustained::at;
// Read-only snapshots. Missing or inconsistent presentation data never invents
// a championship or HFA activation from the week, stadium or camera override.
inline bool conference(Address image,bool neutral,bool rivalry,bool& result){
    Address manager=at<Address>(image+bindings::MatchVisualManager),reference=at<Address>(manager+0x20);
    Address visual=at<Address>(reference+8)&~Address(4);
    if(at<Address>(visual)!=image+bindings::MatchVisualVtable){
        Address live=at<Address>(manager+0x90);visual=at<Address>(live+8)&~Address(4);
    }
    if(at<Address>(visual)!=image+bindings::MatchVisualVtable)return false;
    unsigned char flags[11]{};SIZE_T count=0;
    if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(visual+0x24),flags,sizeof(flags),&count)||count!=sizeof(flags))return false;
    for(auto flag:flags)if(flag>1)return false;
    if((flags[5]!=0)!=neutral||(flags[6]!=0)!=rivalry||(flags[3]&&flags[4]))return false;
    result=flags[3]!=0;return true;
}
inline bool hfa(Address image){
    Address selection=at<Address>(image+bindings::HfaSelection),meter=at<Address>(image+bindings::HfaMeter);
    if(at<Address>(meter)!=image+bindings::HfaMeterVtable||at<Address>(selection)!=image+bindings::HfaSelectionVtable||at<unsigned char>(selection+0x1f0)!=0)return false;
    Address b=at<Address>(meter+0x80),e=at<Address>(meter+0x88);
    if(!b||e<b||e-b>128||(e-b)%8)return false;
    for(Address p=b;p<e;p+=8){
        Address item=at<Address>(p);float weight=at<float>(item+0x18);int tier=at<int>(item+0x1c),side=at<int>(item+0x20);
        if(std::isfinite(weight)&&weight>0&&weight<=1&&tier>=0&&tier<=2&&side==0)return true;
    }
    return false;
}
}
