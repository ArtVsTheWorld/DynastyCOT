#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "GameBindings.h"

namespace attendance::gamecamera {
// Verified from live Zoom/Standard/Wide/All-22/Broadcast selections, including
// mixed offense/defense choices. Do not infer the named preset from its FOV.
enum class Preset:int {Unknown=-1,Zoom=0,Standard=1,Wide=2,All22=3,Broadcast=4};
inline bool known(int p){return p>=0&&p<=4;}
inline const char* name(int p){switch(static_cast<Preset>(p)){case Preset::Zoom:return "Zoom";case Preset::Standard:return "Standard";case Preset::Wide:return "Wide";case Preset::All22:return "All-22";case Preset::Broadcast:return "Broadcast";default:return "unavailable";}}
// Optical normalization supplies the common baseline. These are preferences,
// so 100% has the same meaning for every named camera.
inline constexpr std::array<float,5> Baseline{{1.f,1.f,1.f,1.f,1.f}};
inline float scale(const std::array<float,5>& percent,int p){return known(p)&&std::isfinite(percent[p])?Baseline[p]*(std::clamp)(percent[p],0.f,200.f)*.01f:1.f;}
inline bool custom(const std::array<float,5>& percent){for(float p:percent)if(std::isfinite(p)&&p!=100.f)return true;return false;}
struct Selection {int offense=-1,defense=-1,active=-1;};
template<class Read>inline Selection sample(uintptr_t image,uintptr_t match,Read read){
    Selection result;
    auto load=[&](uintptr_t p,auto& out){return p>=0x10000&&read(p,&out,sizeof(out));};
    uintptr_t optionsObject=0,vt=0;
    if(!load(image+bindings::CameraSettings,optionsObject)||!load(optionsObject,vt)||vt!=image+bindings::CameraSettingsVtable)return result;
    unsigned short offense=0xffff,defense=0xffff;
    if(!load(optionsObject+0x68,offense)||!load(optionsObject+0x6a,defense))return result;
    result.offense=known(offense)?offense:-1;result.defense=known(defense)?defense:-1;
    if(!load(match,vt)||vt!=image+0xb47e908)return result;
    // The two controller records are the existing native team-control contract.
    // Player-lock/RTG and ambiguous mixed-camera multiplayer contexts retain
    // normal motion instead of borrowing a preset from the wrong participant.
    uintptr_t manager=0;
    if(!load(image+bindings::TeamInputManager,manager)||!load(manager,vt)||vt!=image+bindings::TeamInputManagerVtable)return result;
    std::array<int,22> records{};if(!read(manager+0x148,records.data(),sizeof(records)))return result;
    int users=0,side=-1;
    for(int i=0;i<2;++i){int id=records[i*11],team=records[i*11+1],role=records[i*11+5];
        if(id<0)continue;
        if(id>15||team<0||team>1||role!=35)return result;
        ++users;side=team;
    }
    if(!users)return result;
    if(result.offense==result.defense){result.active=result.offense;return result;}
    if(users!=1)return result;
    int possession=-1;if(!load(match+0x1c8,possession)||possession<0||possession>1)return result;
    result.active=possession==side?result.offense:result.defense;return result;
}
template<class Read>inline float preference(const std::array<float,5>& percent,uintptr_t image,uintptr_t match,Read read){
    return custom(percent)?scale(percent,sample(image,match,read).active):1.f;
}
}
