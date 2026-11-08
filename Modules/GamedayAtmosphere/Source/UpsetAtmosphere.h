#pragma once
#include "AttendanceRules.h"
#include "PackedHouse.h"

namespace attendance::upsetsound {
struct Route {
    int stage=0;float discrepancy=0,minimum=1.01f,importance=0;
    bool qualified=false,climax=false;
};
inline const char* name(int stage){return stage==3?"late close upset bid":stage==2?"developing upset":stage==1?"upset watch":"inactive";}
// This is an independent sound admission, never packed-house classification,
// attendance protection, a native rank replacement or a camera venue override.
inline Route evaluate(const State& raw,uint64_t positions,float fullness,bool known,bool enabled=true){
    Route out;
    if(!enabled||!valid(raw)||raw.quarter<2||raw.quarter>4||(raw.phase!=1&&raw.phase!=3&&raw.phase!=4&&raw.phase!=5)||
       positions<512||positions>12800000)return out;
    State s=energyPerspective(raw);
    if(!top25(s.rank1)||s.rank0<=s.rank1)return out;
    out.discrepancy=relativeGap(s);
    // A small rank difference already has the normal ranked-matchup route.
    if(out.discrepancy<.25f)return Route{};
    int deficit=s.awayScore-s.homeScore;
    if(deficit>14||deficit<=-24)return out;
    float finish=s.quarter==4?1-(std::clamp)(s.seconds/(s.quarterSeconds*.25f),0.f,1.f):0;
    // A two-score deficit ceases to be a credible late upset bid.
    if(s.quarter==4&&s.seconds<=s.quarterSeconds*.25f&&deficit>8)return out;
    float factor=s.quarter==2?(deficit<=0?.90f:deficit<=8?.65f:.35f):
        s.quarter==3?(deficit<=0?1.f:deficit<=8?.85f:.40f):
        deficit<=0?1.05f+.20f*finish:deficit<=8?.90f+.20f*finish:.35f;
    out.stage=s.quarter==4&&s.seconds<=s.quarterSeconds*.25f&&std::abs(deficit)<=8?3:
        deficit<=0||(s.quarter>=3&&deficit<=8)?2:1;
    out.minimum=nightRequirement(s,(std::max)(.50f,packed::standardMinimum(positions)-.35f*out.discrepancy*factor));
    out.importance=.20f+.40f*out.discrepancy*factor/1.25f;
    out.qualified=known&&std::isfinite(fullness)&&fullness>=out.minimum&&fullness<=1;
    out.climax=out.stage==3&&out.discrepancy>=.45f;
    if(out.climax)out.importance+=(.12f+.12f*finish)*out.discrepancy;
    return out;
}
}
