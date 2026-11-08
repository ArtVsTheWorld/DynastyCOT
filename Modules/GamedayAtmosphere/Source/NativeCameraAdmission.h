#pragma once
#include "LineupAtmosphere.h"
namespace attendance::nativecamera {
// Published by the same update which decides DGA motion eligibility. This is
// permission to move, not proof that a particular cue is currently playing.
struct Admission {
    uintptr_t match=0;unsigned epoch=0;uint64_t updated=0;
    bool known=false,allowed=false;
};
inline Admission decide(const core::Settings& c,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,
                        float fullness,bool known,uint64_t positions,int type,bool kickoff,bool openingDrive,bool earnedContext){
    Admission a{match,epoch,now,known&&positions>=512&&positions<=12800000&&std::isfinite(fullness)&&fullness>=0&&fullness<=1,false};
    if(!a.known)return a;
    auto lineup=lineup::cameraAdmission(c,p,fullness,true,positions,type,kickoff,openingDrive);
    bool earned=c.enabled&&c.cameraShake&&core::normalLimit(c)>0&&valid(p.game)&&!p.simulated&&
        !energy::clockManagementType(type)&&type>0&&p.game.phase>=3&&p.game.phase<=5&&
        lineup::reactionEligible(p.game,fullness,true,earnedContext);
    a.allowed=lineup.allowed||earned;
    return a;
}
inline bool suppress(const Admission& a,uintptr_t match,unsigned epoch,uint64_t now,float fullness,int minimum,bool dgaPlaying=false){
    // Preserve the advanced INI opt-out and stricter custom floor. Unknown or
    // expired context cannot authorize new context suppression (legacy floor remains).
    if(minimum<=0||!std::isfinite(fullness))return false;
    if(fullness<(std::clamp)(minimum,0,100)*.01f)return true;
    // Admission permits native motion while DGA is idle. A fresh, actually
    // rendered DGA effect owns camera movement instead of adding to HFA.
    return a.match==match&&a.epoch==epoch&&a.known&&now>=a.updated&&now-a.updated<=750&&(!a.allowed||dgaPlaying);
}
}
