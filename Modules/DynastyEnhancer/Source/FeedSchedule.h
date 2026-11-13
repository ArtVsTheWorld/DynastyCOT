#pragma once
#include <cstdint>
namespace dynasty::live {
// Fast identity-only wakeups keep the expensive midweek-content scan at its
// existing five-second interval. Publication requires full capture plus settled proof.
struct FeedSchedule {
    static constexpr uint64_t identityMs=500,contentMs=5000,settleMs=750,retryMs=1000;
    uint64_t pollAt=0,contentAt=0;
    bool due(uint64_t now)const{return now>=pollAt;}
    bool identityOnly(uint64_t now)const{return now<contentAt;}
    void begin(uint64_t now){pollAt=now+retryMs;}
    void observed(uint64_t now){contentAt=0;pollAt=now+settleMs;}
    void accepted(uint64_t now,bool identityAvailable=true){contentAt=now+contentMs;pollAt=now+(identityAvailable?identityMs:contentMs);}
    void unchangedIdentity(uint64_t now){pollAt=now+identityMs;}
    void failed(uint64_t now){contentAt=0;pollAt=now+retryMs;}
    void reset(){pollAt=contentAt=0;}
};
}
