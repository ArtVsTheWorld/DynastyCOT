#pragma once
#include "StartingAttendance.h"
namespace attendance::regeneration {
inline bool eligible(const turnout::Seed& s){return s.dynasty!=0&&!s.resumed&&(s.regular||s.spring)&&s.serial&&s.dynastyGeneration;}
// One-shot, committed only after a verified launch plan succeeds. Repeated On
// polls cannot rearm a consumed request while its INI reset is pending/fails.
struct Request {
    bool pending=false,disarmed=false;uint64_t serial=0,generation=0;int dynasty=0;
    void observe(bool on){if(!on){pending=false;disarmed=false;}else if(!disarmed)pending=true;}
    bool sameLaunch(const turnout::Seed& s)const{return serial&&serial==s.serial&&generation==s.dynastyGeneration&&dynasty==s.dynasty;}
    bool fresh(const turnout::Seed& s)const{return eligible(s)&&(pending||sameLaunch(s));}
    bool commit(const turnout::Seed& s){if(!pending||!eligible(s))return false;pending=false;disarmed=true;serial=s.serial;generation=s.dynastyGeneration;dynasty=s.dynasty;return true;}
};
}
