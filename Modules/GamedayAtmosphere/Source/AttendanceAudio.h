#pragma once
#include "AudioLevels.h"
#include <array>
#include <cstdint>
namespace attendance::balance {
// Retained only for the earned-reaction envelope model, never stacked as a
// separate bus-volume bonus by the production audio callback.
inline float crowdBoost(float f,float maximum=25){return std::isfinite(f)&&std::isfinite(maximum)?(std::clamp)(maximum,0.f,25.f)*(std::clamp)((f-.75f)/.25f,0.f,1.f):0;}
inline bool audiblePhase(int phase,bool unpaused,bool replay){return !replay&&(phase==6||phase==7||(unpaused&&phase>=1&&phase<=5));}

inline constexpr std::array<unsigned short,8> Groups={9,12,13,14,15,16,17,18};
inline constexpr std::array<const char*,8> Names={"Crowd","Anticipation_Home_Pass","Beds","Chants/Response","Reactions","StudentSection","Sweeteners","Swells"};
inline float smooth(float x){x=(std::clamp)(x,0.f,1.f);return x*x*(3-2*x);}
inline float quarter(float x){return std::round(x*4)*.25f;}
// Signal-domain monotone Hermite curve: finite nonzero floor, a 50% bed at
// 25%-full smallest venue, and zero slope at the vanilla crossover.
inline float sparseProgress(float r){
 constexpr float x=5.f/14,y=1.f/6;
 r=(std::clamp)(r,0.f,1.f);
 const float d0=y/x,d1=(1-y)/(1-x),w0=2*(1-x)+x,w1=(1-x)+2*x;
 const float slope=(w0+w1)/(w0/d0+w1/d1);
 float h,t,v0,v1,m0,m1;
 if(r<x){h=x;t=r/h;v0=0;v1=y;m0=0;m1=slope;}
 else{h=1-x;t=(r-x)/h;v0=y;v1=1;m0=slope;m1=0;}
 float t2=t*t,t3=t2*t;
 return (std::clamp)((2*t3-3*t2+1)*v0+(t3-2*t2+t)*h*m0+(-2*t3+3*t2)*v1+(t3-t2)*h*m1,0.f,1.f);
}
inline constexpr std::array<float,7> SignalFloors={.55f,.40f,.40f,.55f,.40f,.95f,.40f};
inline float venueWeight(uint64_t seats){return (std::clamp)(std::log(static_cast<float>((std::max)(seats,uint64_t(6060)))/6060.f)/std::log(61888.f/6060.f),0.f,1.f);}
inline float crossover(uint64_t seats){return .70f-.40f*venueWeight(seats);}
inline float denseBackground(float fullness,uint64_t seats){
 float occupied=static_cast<float>(seats)*fullness;
 return smooth((fullness-.80f)/.12f)*smooth((occupied-20000.f)/7000.f);
}
inline float backgroundCeiling(float fullness,uint64_t seats){
 float occupied=static_cast<float>(seats)*fullness;
 return 3.5f+.5f*smooth((occupied-45000.f)/(61888.f-45000.f));
}
struct Mix {
 std::array<float,8> gain{};std::array<int,9> key{};bool valid=false,mute=false;
 float net(unsigned child)const{return gain[0]+gain[child+1];}
 bool needed()const{if(mute)return true;for(float db:gain)if(db!=0)return true;return false;}
};
// Fullness and mapped occupied positions determine density and the crossover.
// Sparse anchors are listening choices, not measured acoustic statistics.
inline Mix mix(float fullness,uint64_t seats,float maximumPercent=30,float strengthPercent=100,bool hostileOpening=false){
 Mix out;if(!std::isfinite(fullness)||fullness<0||seats<512||seats>12800000||!std::isfinite(maximumPercent)||!std::isfinite(strengthPercent))return out;
 out.valid=true;fullness=(std::min)(fullness,1.f);maximumPercent=(std::clamp)(maximumPercent,0.f,30.f);
 if(fullness==0){out.mute=true;out.gain[0]=-60;}
 else {
  std::array<float,7> net{},ceiling{};ceiling.fill(1+maximumPercent*.01f);float point=crossover(seats);
  if(fullness<point){
   float progress=sparseProgress(fullness/point);
   for(unsigned i=0;i<7;++i)net[i]=20*std::log10(SignalFloors[i]+(1-SignalFloors[i])*progress);
  }else {float lift=(std::min)(maximumPercent*.01f,.15f+.15f*venueWeight(seats));float db=20*std::log10(1+lift*smooth((fullness-point)/(1-point)));net.fill(db);}
  // Listening-approved dense backgrounds use a larger native bed/chant
  // ceiling, not an extra loop or reaction gain. Ordinary/sparse mixes stay
  // exact. The existing advanced boost setting scales both ceilings.
  float presence=denseBackground(fullness,seats),background=backgroundCeiling(fullness,seats)*maximumPercent/30.f;
  for(unsigned i:{1u,2u,4u}){
   net[i]+=(std::max)(0.f,background-net[i])*presence;
   ceiling[i]=(std::max)(ceiling[i],std::pow(10.f,background/20));
  }
  // A .001 signal-gain lattice bounds preset churn while native .25s fades
  // smooth transitions. Floors and the 50% signal anchor stay exact.
  float strength=(std::clamp)(strengthPercent,0.f,100.f)*.01f;
  // A hostile first offensive call gets a fuller bed/chant and immediate swell.
  // Admission remains with the opening-drive model; 0 still means native mix.
  if(hostileOpening){for(unsigned i:{1u,2u,4u,6u}){
   float hostileDb=(i==6?3.5f:6.f)*maximumPercent/30.f;net[i]=(std::max)(net[i],hostileDb);
   ceiling[i]=(std::max)(ceiling[i],std::pow(10.f,hostileDb/20));
  }}
  for(unsigned i=0;i<7;++i){
   float gain=std::pow(10.f,net[i]*strength/20);
   gain=(std::clamp)(std::round(gain*1000)/1000,std::pow(SignalFloors[i],(std::min)(strength,1.f)),(std::min)(std::pow(10.f,8.f/20),std::pow(ceiling[i],strength)));
   net[i]=20*std::log10(gain);out.gain[0]=(std::max)(out.gain[0],net[i]);
  }
  for(unsigned i=0;i<7;++i)out.gain[i+1]=net[i]-out.gain[0];
 }
 for(unsigned i=0;i<8;++i)out.key[i]=static_cast<int>(std::lround(out.gain[i]*10000));
 out.key[8]=out.mute?1:0;return out;
}
// Only continuous bed/chant/student buses are softened. Parent, reactions,
// anticipation, sweeteners and swells retain their exact net levels. Native
// quarter-second mixer fades remain in control. Half-dB steps bound churn.
inline Mix ambient(Mix out,float signalGain){
 if(!out.valid||out.mute||!std::isfinite(signalGain)||signalGain>=1||signalGain<0)return out;
 float db=(std::clamp)(.5f*std::round(40*std::log10((std::max)(signalGain,.20f))),-12.f,0.f);
 for(unsigned child:{1u,2u,4u}){out.gain[child+1]+=db;out.key[child+1]=static_cast<int>(std::lround(out.gain[child+1]*10000));}
 return out;
}
}
