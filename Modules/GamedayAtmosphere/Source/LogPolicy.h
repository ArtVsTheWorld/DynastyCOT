#pragma once
#include <string>
namespace attendance {
inline bool importantLogTag(const std::string& kind,const std::string& message){
    if(kind.rfind("spring-",0)==0||kind=="crowd-history")return true;
    if(kind=="home-crowd"||kind=="home-crowd-proof"||kind=="home-crowd-envelope"||kind=="visitor-cheer-gate"||kind=="visiting-section")return true;
    if(kind=="replay-route"||kind=="visitor-demand-components"||kind=="away-appearance-budget"||kind=="starting-attendance-state")return true;
    if(kind.find("unavailable")!=std::string::npos||kind=="2020-mode"||kind=="defensive-motion"||kind=="showboat"||kind=="audience-history")return true;
    return !(kind!="defensive-proof"&&kind!="major-replay"&&kind!="replay-select"&&kind!="starting"&&kind!="ready"&&kind!="inactive"&&kind!="fault"&&kind!="quarter-break"&&
       kind!="two-minute"&&kind!="departures"&&kind!="arrivals"&&kind!="halftime"&&kind!="opening"&&kind!="eruption"&&
       kind!="matchup"&&kind!="roar"&&kind!="late-score"&&kind!="protection"&&kind!="checkpoint-skipped"&&kind!="new-game"&&kind!="requested"&&
       kind!="diagnostics"&&kind!="reaction-audio"&&kind!="play-summary"&&kind!="late-turnover"&&kind!="opening-kickoff"&&kind!="defensive-reactions-unavailable"&&kind!="configuration"&&
       kind!="binding-mismatch"&&kind!="audio-state"&&kind!="live-play"&&kind!="resumed-game"&&kind!="replay"&&kind!="history"&&kind!="replay-unavailable"&&kind!="seating"&&kind!="seating-density"&&
       kind!="pass-flight"&&kind!="upset-watch"&&kind!="pause-menu"&&kind!="settings-menu"&&kind!="starting-attendance"&&kind!="cannon"&&kind!="away-motion"&&kind.find("inactive")==std::string::npos&&!(kind=="audio"&&message.find("fault")!=std::string::npos));
}
}
