#pragma once
#include <algorithm>
#include <cmath>
namespace attendance::visitormodel {
inline double smooth(double x){x=(std::clamp)(x,0.,1.);return x*x*(3-2*x);}
// Native TeamPrestige is 0..10 in half-star units. No attendance-derived reach.
inline double national(double prestige){
 if(!std::isfinite(prestige)||prestige<7||prestige>10)return 0;
 constexpr double anchors[]={.12,.40,.70,1.};
 int i=(std::min)(2,static_cast<int>(prestige)-7);double t=smooth(prestige-7-i);
 return anchors[i]+(anchors[i+1]-anchors[i])*t;
}
inline double travel(double miles){
 if(!std::isfinite(miles)||miles<0)return 1;
 constexpr double x[]={0,150,400,800,1500,2500,5000};
 constexpr double y[]={2.05,1.70,1.18,.78,.32,.16,.10};
 for(unsigned i=1;i<7;++i)if(miles<=x[i])return y[i-1]+(y[i]-y[i-1])*smooth((miles-x[i-1])/(x[i]-x[i-1]));
 return .10;
}
inline double form(int wins,int losses){int n=wins>=0&&wins<=30&&losses>=0&&losses<=30?wins+losses:0;return n>0&&n<=30?double(wins)/n:.5;}
inline double confidence(int wins,int losses){if(wins<0||wins>30||losses<0||losses>30||wins+losses>30)return 0;return (std::clamp)((wins+losses)/6.,0.,1.);}
inline double season(int prestige,int wins,int losses){
 double f=form(wins,losses),c=confidence(wins,losses),expected=.42+.035*(std::clamp)(prestige,0,10);
 return (std::clamp)(1+c*(.90*(f-.5)-.65*(std::max)(0.,expected-f)),.30,1.35);
}
struct Inputs {
 double homeFans=0,awayFans=0,homeTickets=0,travelFactor=1;
 int homePrestige=-1,awayPrestige=-1,homeWins=-1,homeLosses=-1,awayWins=-1,awayLosses=-1;
 bool rivalry=false,neutral=false,fcs=false;unsigned seed=0;
};
struct Components {bool valid=false;double nationalReach=0,regional=0,local=0,national=0,home=0,visitors=0,share=0,ceiling=.18,takeover=0;};
inline Components demand(const Inputs& d,double slider=100,bool lowEndLift=false){
 Components c;
 if(!std::isfinite(d.homeFans)||d.homeFans<=0||!std::isfinite(d.homeTickets)||d.homeTickets<=0||
   (!d.fcs&&(!std::isfinite(d.awayFans)||d.awayFans<=0))||d.homePrestige<0||d.homePrestige>10||(!d.fcs&&(d.awayPrestige<0||d.awayPrestige>10)))return c;
 c.valid=true;c.home=d.homeTickets;
 double away=d.fcs?(std::clamp)(d.homeFans*.30,6000.,24000.):d.awayFans;
 c.nationalReach=d.fcs?0:national(d.awayPrestige);
 double excitement=season(d.awayPrestige,d.awayWins,d.awayLosses);
 double proximity=std::isfinite(d.travelFactor)?(std::clamp)(d.travelFactor,.1,2.05):1.;
 // Local/national support is modeled, not a claim about actual alumni addresses.
 c.local=away*.050*(.85+.15*c.nationalReach)*(.80+.20*excitement);
 c.national=4700*c.nationalReach*(.80+.20*excitement);
 c.regional=away*.055*proximity*excitement*(1+.35*c.nationalReach*smooth((1-proximity)/.9));
 double rivalry=d.rivalry?1.15:1.;
 double variety=.97+.06*((d.seed*1664525u+1013904223u)%10001)/10000.;
 c.visitors=(c.local+c.national+c.regional)*rivalry*variety;
 if(d.neutral){
  double h=d.homeFans*(.35+.65*national(d.homePrestige))*(.85+.15*season(d.homePrestige,d.homeWins,d.homeLosses));
  double a=away*(.35+.65*c.nationalReach)*(.85+.15*excitement);
  c.share=(std::clamp)(a/(h+a),.30,.70);c.visitors=c.home*c.share/(1-c.share);c.ceiling=.70;return c;
 }
 double struggle=confidence(d.homeWins,d.homeLosses)*smooth((.50-form(d.homeWins,d.homeLosses))/.25);
 double weak=smooth((.85-c.home/d.homeFans)/.30);
 double good=confidence(d.awayWins,d.awayLosses)*smooth((form(d.awayWins,d.awayLosses)-.50)/.25);
 c.takeover=struggle*weak*good*smooth((c.nationalReach-.12)/.58);c.ceiling=.18+.17*c.takeover;
 double strength=std::isfinite(slider)?(std::clamp)(slider,0.,200.)/100.:1.;
 c.visitors*=strength;
 double raw=c.visitors/(c.home+c.visitors),core=(std::min)(raw,.09);
 // Ordinary ticket access resists the extra contingent smoothly. A fully
 // qualified struggling host can release that resistance, never prestige alone.
 double ordinary=core+.09*(1-std::exp(-(raw-core)/.14));
 // New-plan calibration only: a tapered lift around ordinary 5% support,
 // practically zero at healthy 12-16% shares; no new universal floor.
 if(lowEndLift)ordinary+=.028*std::exp(-std::pow((ordinary-.05)/.035,2));
 c.share=(std::clamp)(ordinary+(raw-ordinary)*c.takeover,.03,c.ceiling);
 c.visitors=c.home*c.share/(1-c.share);return c;
}
}
