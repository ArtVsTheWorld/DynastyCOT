#pragma once
#include <cstdint>
namespace attendance {
inline unsigned fanHash(unsigned agent){unsigned h=agent*747796405u+2891336453u;h=((h>>((h>>28)+4))^h)*277803737u;return (h>>22)^h;}
inline bool departed(unsigned agent,int loss){return (fanHash(agent)&0xffffu)<static_cast<unsigned>(loss*65536u/100u);}
inline bool hidePart(bool nativeHidden,bool startingHidden,bool currentlyHidden,bool leaving,bool restore,bool arriving=false,bool temporaryOwned=false,bool firstNativeInput=false){
    if(firstNativeInput)currentlyHidden=nativeHidden;
    return nativeHidden||startingHidden||(!restore&&((currentlyHidden&&!(arriving&&temporaryOwned))||leaving));
}
// Replay/initialization may restore our own empty seats. Native empty seats
// and the original attendance target remain authoritative in both directions.
inline bool historicalPart(bool nativeHidden,bool startingHidden,unsigned agent,int loss){
    return nativeHidden||startingHidden||departed(agent,loss);
}
inline bool historicalSelection(bool nativeHidden,bool startingHidden,bool selected){return nativeHidden||startingHidden||selected;}
struct VisibilityCount {
    unsigned long long original=0,visible=0,expected=0,nativeExtra=0,otherExtra=0,extraVisible=0;
};
inline void countSelection(VisibilityCount& c,unsigned short parts,unsigned short starting,unsigned short current,unsigned short native,bool selected){
    if(!parts||(starting&parts)==parts)return;
    ++c.original;bool visible=(current&parts)!=parts;
    if(visible)++c.visible;
    if(!selected){
        ++c.expected;
        if(!visible){if((native&parts)==parts)++c.nativeExtra;else ++c.otherExtra;}
    }else if(visible)++c.extraVisible;
}
inline void countFan(VisibilityCount& c,unsigned agent,unsigned short parts,unsigned short starting,unsigned short current,unsigned short native,int loss){
    countSelection(c,parts,starting,current,native,departed(agent,loss));
}
}
