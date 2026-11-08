#pragma once
#include "CoreAtmosphere.h"
#include "LineupAtmosphere.h"
namespace attendance::overtimeeffects {
inline bool qualified(const core::Settings& c,const energy::Play& p,float fullness,bool known,uint64_t seats,bool allowed=true){
    return allowed&&known&&c.enabled&&c.overtime&&c.lineups&&core::normalLimit(c)>0&&valid(p.game)&&
        overtime(p.game)&&std::abs(p.game.homeScore-p.game.awayScore)<=8&&!p.simulated&&
        std::isfinite(fullness)&&fullness>=nightRequirement(p.game,packed::specialMinimum(seats));
}
// A sound-only floor across OT kick/try/selection gaps. Existing result,
// return and winning envelopes keep priority; this creates no fake play.
inline sustained::Intent fill(sustained::Intent out,const core::Settings& c,const energy::Play& p,float fullness,bool known,uint64_t seats,bool allowed){
    if(!qualified(c,p,fullness,known,seats,allowed)||p.game.phase<1||p.game.phase>5||out.priority>=3)return out;
    out.active=true;out.reason=sustained::Reason::Overtime;out.priority=2;out.continuous=true;out.cheers=false;
    out.lineup=(uint64_t{1}<<62)|1;out.intensity=(std::max)(out.intensity,900.f*std::sqrt((std::clamp)(fullness/.8f,0.f,1.f)));
    out.size=fullness>=.8f?3:2;out.supporting=true;out.attackMs=700;out.releaseMs=3500;
    return out;
}
}
