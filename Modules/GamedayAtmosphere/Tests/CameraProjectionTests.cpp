#include "../Source/DirectorShake.h"
#include <cassert>
#include <iostream>
#include <limits>
using namespace attendance::motion;
using namespace attendance::motion::projection;
std::array<float,16> lens(float degrees,float aspect){
    std::array<float,16> p{};p[5]=1/std::tan(degrees*.00872664626f);p[0]=p[5]/aspect;p[11]=1;return p;
}
std::array<float,2> project(const std::array<float,16>& v,const std::array<float,16>& p){
    // Project a fixed world point straight ahead using the resulting view.
    float x=10*v[8]+v[12],y=10*v[9]+v[13],z=10*v[10]+v[14];
    return {p[0]*x/z*.5f,p[5]*y/z*.5f};
}
int main(){
    Cue cue{99,1,2000,1000,10000,Kind::LivePlay,10,false};
    const std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    unsigned checked=0,framed=0;float maximumRelativeError=0,maximumFramedError=0;
    for(float level:{2.f,10.f,30.f})for(float setting:{.5f,1.f,2.f})for(uint64_t tick:{2000u,2017u,2031u,2046u}){
        cue.percent=level;cue.updated=tick;auto reference=lens(35,16.f/9),referenceView=identity;
        assert(apply(referenceView,cue,tick,1,1,ReferenceStrength*setting));auto target=project(referenceView,reference);
        for(float fov:{15.f,25.f,35.f,50.f,70.f,88.5f,100.f})for(float aspect:{4.f/3,16.f/9,21.f/9}){
            auto p=lens(fov,aspect);auto s=scale(p);assert(s.valid);
            if(s.clipped){assert(s.pitch<=4&&s.yaw<=4);continue;}
            auto view=identity;assert(apply(view,cue,tick,s.pitch,s.yaw,ReferenceStrength*setting));auto actual=project(view,p);
            // Combined-axis rotation/roll has small perspective cross-terms;
            // the independently projected movement stays close to reference.
            assert(std::fabs(actual[0]-target[0])<.0008f&&std::fabs(actual[1]-target[1])<.0008f);
            float dx=actual[0]-target[0],dy=actual[1]-target[1];
            float relative=std::sqrt((dx*dx+dy*dy)/(target[0]*target[0]+target[1]*target[1]));
            maximumRelativeError=(std::max)(maximumRelativeError,relative);assert(relative<.05f);
            assert(validMatrix(view));++checked;
            // Optical normalization still works with the intentional framing
            // preference. Compare against the same biased reference, not an
            // equal-strength view, so a double-applied bias fails this check.
            referenceView=identity;view=identity;
            assert(apply(referenceView,cue,tick,1,1,ReferenceStrength*setting*s.framing));
            assert(apply(view,cue,tick,s.pitch,s.yaw,ReferenceStrength*setting*s.framing));
            target=project(referenceView,reference);actual=project(view,p);
            dx=actual[0]-target[0];dy=actual[1]-target[1];
            relative=std::sqrt((dx*dx+dy*dy)/(target[0]*target[0]+target[1]*target[1]));
            maximumFramedError=(std::max)(maximumFramedError,relative);assert(relative<.05f);++framed;
            referenceView=identity;assert(apply(referenceView,cue,tick,1,1,ReferenceStrength*setting));target=project(referenceView,reference);
        }
    }
    // Broad framing folds in the accepted 2.0.34 90% preference; tight
    // framing keeps 2.0.33. Both use the 2.0.31 common reference.
    assert(std::fabs(ReferenceStrength-1.10f)<.000001f);
    assert(std::fabs(framingGain(1.9437481f)-.675f)<.000001f);
    assert(std::fabs(framingGain(2.9128082f)-1.05f)<.000001f);
    assert(std::fabs(framingGain((1.9437481f+2.9128082f)*.5f)-.8625f)<.000001f);
    unsigned curveChecks=0;float previous=1.05f;
    for(unsigned step=500;step<=16000;++step){
        const float fov=step*.01f;auto p=lens(fov,16.f/9);auto s=scale(p);if(!s.valid)continue;
        assert(s.framing>=.675f&&s.framing<=1.050001f&&s.framing<=previous+.000001f);
        assert(previous-s.framing<.001f);previous=s.framing;
        for(float aspect:{4.f/3,21.f/9}){auto other=scale(lens(fov,aspect));if(other.valid)assert(other.framing==s.framing);}
        ++curveChecks;
    }
    // The flat endpoints meet the transition without a slope jump.
    for(float focal:{1.9437481f,2.9128082f})assert(std::fabs(framingGain(focal+.0001f)-framingGain(focal-.0001f))<.000001f);
    for(auto kind:{Kind::Ordinary,Kind::Critical,Kind::LivePlay,Kind::Touchdown,Kind::Eruption,Kind::Safety,Kind::BlockedKick,Kind::Clincher,Kind::Cannon}){
        Cue tested{99,1,2000,1000,17000,kind,20,false,30};
        for(float fov:{35.f,54.449f,37.896f,88.5f}){
            auto p=lens(fov,16.f/9);auto optical=scale(p);auto accepted=identity,current=identity;
            const bool tight=fov==35.f||fov==37.896f;
            const float requested=tight?1.05f:.675f;
            assert(std::fabs(optical.framing-requested)<.000001f);
            // Independent historical gains: 2.0.33 tight / 2.0.34 at90% broad.
            assert(apply(accepted,tested,2000,optical.pitch,optical.yaw,tight?1.155f:(.825f*.90f)));
            assert(apply(current,tested,2000,optical.pitch,optical.yaw,ReferenceStrength*optical.framing));
            for(size_t i=0;i<current.size();++i)assert(std::fabs(current[i]-accepted[i])<.000001f);
        }
    }
    auto extreme=lens(170,16.f/9);assert(!scale(extreme).valid);extreme=lens(150,16.f/9);
    assert(scale(extreme).clipped&&scale(extreme).pitch==4&&scale(extreme,true).pitch==6);
    auto invalid=lens(35,16.f/9);invalid[0]=std::numeric_limits<float>::quiet_NaN();assert(!scale(invalid).valid);
    History history;auto normal=lens(35,16.f/9);std::array<float,16> missing{};
    assert(!history.observe(1,2,3,4,100,missing).valid);
    assert(history.observe(1,2,3,4,100,normal).valid);
    assert(history.observe(1,2,3,4,350,missing).cached);
    assert(history.observe(1,2,3,4,350,missing).framing==scale(normal).framing);
    assert(!history.observe(1,2,3,4,351,missing).valid);
    assert(!history.observe(1,2,3,4,99,missing).valid);
    assert(!history.observe(2,2,3,4,110,missing).valid);
    assert(!history.observe(1,5,3,4,110,missing).valid);
    assert(!history.observe(1,2,5,4,110,missing).valid);
    assert(!history.observe(1,2,3,5,110,missing).valid);
    auto wide=lens(88.5f,16.f/9);auto current=history.observe(1,2,3,4,110,wide);
    assert(!current.cached&&std::fabs(current.pitch-ReferenceY/wide[5])<.00001f);
    for(uintptr_t view=10;view<30;++view)history.observe(1,view,3,4,120,normal);
    assert(!history.observe(1,2,3,4,121,missing).valid); // Fixed-size eviction.
    cue.updated=cue.began=2000;cue.percent=0;auto output=identity;assert(!apply(output,cue,2000)&&output==identity);
    std::cout<<"PASS: "<<checked<<" optical and "<<framed<<" framed camera comparisons; maximum center-point displacement error "<<maximumRelativeError*100<<"% / "<<maximumFramedError*100<<"%; "<<curveChecks<<" bounded monotonic framing samples; effect/preferences/phase, extreme bounds, projection leases, cache eviction and zero output.\n";
}
