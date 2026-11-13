#pragma once
#include "StartingAttendance.h"
namespace attendance::spring {
inline turnout::Result audienceTurnout(const turnout::Seed& s,const turnout::Launch& l){
 turnout::Result r;r.attendance=l.ea;
 const auto& actual=s.springSide==0?s.home:s.away;
 if(!s.spring||s.resumed||!s.dynasty||actual.id!=s.springSchool||!actual.prestigeKnown||l.ea<=0||l.capacity<1000||l.capacity>200000||
    !std::isfinite(actual.audienceFans)||actual.audienceFans<=0||!l.weatherKnown||l.minutes<0||l.minutes>=1440||l.precipitation<0||l.precipitation>3)return r;
 // Reuse verified program audience, without an opponent, rankings, rivalry,
 // senior-day, home-opener, record momentum or Fuller Crowds multiplier.
 r.audienceModel=true;r.audienceFans=actual.audienceFans;r.night=l.minutes>=1140;
 // Peer exhibition conversion, not a second prestige/record multiplier.
 const double pressure=actual.audienceFans/l.capacity;
 const double q=(std::clamp)((pressure-.4)/1.6,0.0,1.0);
 const double engagement=.30+.29*q*q*(3-2*q);
 const unsigned salt=turnout::fixtureHash(s)^0x53505247u;
 const double variation=(static_cast<int>(salt%1001)-500)/100000.0; // +/-0.5 capacity point.
 const double target=(std::min)(actual.audienceFans,l.capacity*(engagement+variation));
 const double Exhibition=target/actual.audienceFans;
 const double time=r.night?1.05:1.0;
 constexpr double weather[]={1,.95,.90,.82};
 const double wet=s.roof>=1&&s.roof<=3?1:weather[l.precipitation];
 const double planned=actual.audienceFans*Exhibition*time*wet;
 r.demand=static_cast<float>(100*actual.audienceFans/l.capacity);r.eventMultiplier=static_cast<float>(Exhibition*time);
 r.weather=static_cast<float>(100*actual.audienceFans*Exhibition*time*(wet-1)/l.capacity);r.capacityLimit=60;
 r.attendance=(std::clamp)(static_cast<int>(std::lround(planned)),1,l.capacity*60/100);
 r.percent=100.f*r.attendance/l.capacity;r.kickoff=80;r.fillPercent=10;r.applied=true;return r;
}
}
