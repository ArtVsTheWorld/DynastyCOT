#include "../Source/CrowdAudioModel.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
static void near(float a,float b){assert(std::fabs(a-b)<0.00001f);}
int main(){
    using BaseGain=attendance::audio::Gain;
    struct Gain:BaseGain {void attendance(float fraction,float strength=.2f,float emphasis=1,float arrivals=1,float start=.8f){BaseGain::attendance(fraction,strength,emphasis,arrivals,start);}};
    // Retain the configurable 80-percent curve's transport/round-trip coverage.
    // AttendanceAudioTests covers the new production default at40 percent.
    // Independent acceptance table: percentage of the native signal retained.
    const float attendance[]={1,.9f,.8f,.7f,.6f,.5f,.4f,.35f,.3f,.2f,.15f,.1f};
    const float levels[]={1,1,1,.975f,.95f,.925f,.9f,.8875f,.875f,.85f,.8375f,.825f};
    for(int i=0;i<12;++i){Gain row;row.attendance(attendance[i],.2f,1,1,.8f);near(row.target(),levels[i]);row.advance(6);near(row.value(),levels[i]);}
    Gain gain;near(gain.linear(.8f),.8f);gain.attendance(.4f);
    near(gain.advance(3),.95f);gain.attendance(.4f);near(gain.advance(3),.9f); // Identical callbacks do not restart the fade.
    near(gain.linear(0),0);near(gain.linear(.8f),.72f);near(gain.linear(1.6f),1.44f);
    near(gain.decibels(-15),-15.91515f);near(gain.decibels(-60),-60.91515f);
    for(int i=0;i<300;++i){gain.attendance(.4f);gain.advance(1.0f/60);}near(gain.linear(.8f),.72f);
    gain.attendance(.9f);near(gain.target(),.9f); // Permanent crowd loss never refills on a comeback.
    gain.attendance(std::numeric_limits<float>::quiet_NaN());near(gain.target(),.9f);
    gain.advance(std::numeric_limits<float>::infinity());near(gain.value(),.9f);gain.advance(-1);near(gain.value(),.9f);
    gain.attendance(0);gain.advance(6);near(gain.linear(1),0); // Truly empty is still silent.
    near(gain.decibels(-15),-96);near(gain.decibels(-120),-120);
    gain.restore();near(gain.advance(3),.5f);near(gain.advance(3),1);
    gain.reset();near(gain.linear(.8f),.8f);gain.attendance(2);near(gain.target(),1);gain.attendance(-1);near(gain.target(),0);
    Gain a,b;a.attendance(.22f);b.attendance(.22f);a.advance(6);for(int i=0;i<600;++i)b.advance(.01f);near(a.value(),b.value());
    Gain gentle;gentle.attendance(.4f,.4f);gentle.advance(6);near(gentle.value(),.8f);
    gentle.attendance(.8f,.4f);near(gentle.target(),.8f);gentle.attendance(.4f,0);gentle.advance(6);near(gentle.value(),1);
    gentle.attendance(0,0);near(gentle.target(),1);gentle.attendance(0,.4f);gentle.advance(6);near(gentle.value(),0);
    gentle.restore();gentle.advance(6);gentle.attendance(1,.4f);near(gentle.target(),1);
    gentle.attendance(.4f,std::numeric_limits<float>::quiet_NaN());near(gentle.target(),1);
    Gain arriving;arriving.attendance(1,.2f,1,.6f);arriving.advance(6);near(arriving.value(),.95f);
    arriving.attendance(1,.2f,1,1);arriving.advance(6);near(arriving.value(),1); // Authorized Q1 arrivals may restore volume.
    arriving.attendance(.7f,.2f,1,1);arriving.advance(6);arriving.attendance(1,.2f,1,1);near(arriving.target(),.975f);
    Gain custom;custom.attendance(.8f,.2f,1,1,.9f);near(custom.target(),.9777778f);
    Gain boost;boost.attendance(.1f,.2f,1.35f);near(boost.target(),1); // Attenuation never exceeds native slider.
    Gain calibrated;calibrated.attendance(.1f);calibrated.advance(6);near(calibrated.decibels(0),-1.670921f);
    for(int fullness=0;fullness<=100;++fullness){
        Gain bounded;bounded.sparse(fullness*.01f,.99f,.5f);bounded.advance(6);
        float expected=fullness>=50?1:.75f+fullness*.005f;near(bounded.value(),expected);
        near(bounded.linear(0),0);near(bounded.linear(.8f),expected*.8f);
    }
    Gain smooth;smooth.sparse(.1f);near(smooth.advance(3),.9f);smooth.sparse(.1f);near(smooth.advance(3),.8f);
    smooth.sparse(.4f);near(smooth.advance(6),.95f); // Current density can recover with arrivals or a historical clip.
    smooth.sparse(0,0);near(smooth.advance(6),1);
    std::puts("PASS: 80% threshold, percentage acceptance table, six-second fades, no compounding/refill, arrivals, silent empty crowd and native slider preservation.");
}
