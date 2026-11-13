#pragma once
#include <algorithm>
namespace attendance::spring {
// Extensions has two observed paths: ordinary PreSeason exhibition rows and
// a launched game 250 carrying its Spring Situations stamp on the stand-in.
// 1100000/1200000 are observed in the current 0.3.0 native BeginGame logs;
// The base 1000000 is observed only on a PreSeason launch; its caller
// additionally requires actual-school tracking AND ownership of the venue.
// Unknown stamps stay native. An unplayed marker 250 alone is never enough.
inline bool launchedStamp(int type,int number,int home,int stamp){return number==250&&home==1502&&((stamp==1000000&&type==0)||((stamp==1100000||stamp==1200000)&&(type==0||type==1)));}
inline bool baseEvidence(int stamp,int side,bool tracked,bool venue){return stamp!=1000000||(side==1&&tracked&&venue);}
inline int actualSide(int type,int number,bool practice,bool bowl,int home,int away,bool homeSchool,bool awaySchool,int stamp=-1){
    const bool stamped=launchedStamp(type,number,home,stamp);
    if((!stamped&&(type!=0||number==250))||number<0||practice||bowl||home==away||home<0||away<0)return -1;
    const auto synthetic=[](int id){return id>=1500&&id<=1504;};
    if(homeSchool&&!synthetic(home)&&synthetic(away))return 0;
    if(awaySchool&&!synthetic(away)&&synthetic(home))return 1;
    return -1;
}
inline int turnout(int native,int capacity,bool resumed=false){
    return !resumed&&native>0&&capacity>=1000&&capacity<=200000?(std::min)(native,capacity*60/100):native;
}
}
