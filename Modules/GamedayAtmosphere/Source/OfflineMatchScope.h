#pragma once
namespace attendance::scope {
// Observed native mode IDs. Mode 17 covers both Play Now and main-menu
// practice; the configuration flag must distinguish them at every admission.
inline bool matchMode(int mode){return mode==14||mode==17;}
inline bool offlineMatch(int mode,int session,int practice){
    return matchMode(mode)&&session==3&&practice==0;
}
inline bool dynasty(int mode){return mode==14;}
inline bool playNow(int mode){return mode==17;}
}
