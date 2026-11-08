#include "../Source/DefensiveAccents.h"
#include "../Source/MotionSoundPairing.h"
#include <cassert>
#include <cstdio>
#include <unordered_map>
using namespace attendance;
using A=uintptr_t;
std::unordered_map<A,unsigned char> memory;
template<class T>void put(A p,T value){for(size_t n=0;n<sizeof(T);++n)memory[p+n]=reinterpret_cast<unsigned char*>(&value)[n];}
bool torn=false;
bool read(A p,void* out,size_t size){
    for(size_t n=0;n<size;++n){if(!memory.count(p+n))return false;static_cast<unsigned char*>(out)[n]=memory[p+n];}
    if(torn&&size==32)static_cast<unsigned char*>(out)[16]^=1;
    return true;
}
struct Case {
    defense::Model model;energy::Play p,snap;energy::Outcome result;defense::Sample sample{999,true,0,-1};
    A match=123;unsigned epoch=1;uint64_t now=1000,sequence=1,prior=0;bool allowed=true,held=false;int kind=3;
    Case(int down=3,int side=1,bool neutral=false){p.game={2,14,14,1,300,720};p.game.neutral=neutral;p.down=down;p.offense=side;snap=p;tick(1);tick(3);}
    live::Outcome tick(int phase,live::Outcome current={}){now+=100;p.game.phase=phase;return model.observe(p,snap,result,match,epoch,sequence,now,allowed,sample,kind,held,current,prior);}
    void tip(){sample.deflection=412.8999753892422;sample.side=1-snap.offense;assert(!tick(3).id);}
    void stopped(){result.accepted=result.completed=true;result.sequence=sequence;p.down=4;if(snap.down==4){p.offense=1-snap.offense;p.down=1;}}
};
int main(){
    // Independent layout from the captured third-and-20 swat: type 39,
    // actor 0x80004, home defender; the knockout byte was zero throughout.
    constexpr A image=0x140000000,root=0x100000,actors=0x110000,buckets=0x120000,node=0x130000,array=0x140000,actor=0x150000,data=0x160000;
    put(image+skills::EventRoot,root);put(root,image+0xbc90128);for(unsigned n=0;n<45*32;++n)put(root+0x10+n,(unsigned char)0);
    put(root+0x10+16,412.8999753892422);put(root+0x10+24,39);put(root+0x10+28,0x80004u);
    put(image+skills::ActorRoot,actors);put(actors+0x170,buckets);put(actors+0x178,1u);put(actors+0x28,array);put(actors+0x30,array+8);
    put(buckets,node);put(node,0x80004u);put(node+4,0);put(array,actor);put(actor+0x18,data);put(data+0x44,(unsigned char)1);
    put(data+0x304,0);put(data+0x308,8);put(data+0x310,100);
    auto native=defense::snapshot(image,read);assert(native.valid&&native.object==root&&native.side==0&&native.deflection>412);
    assert(defense::snapshot(image,read,413).side==-1);put(data+0x304,1);assert(defense::snapshot(image,read).side==-1);put(data+0x304,0);
    put(root,A(0));assert(!defense::snapshot(image,read).valid);put(root,image+0xbc90128);
    torn=true;assert(!defense::snapshot(image,read).valid);torn=false;
    put(root+0x10+16,std::numeric_limits<double>::quiet_NaN());assert(defense::snapshot(image,read).deflection==0);put(root+0x10+16,412.8999753892422);
    // Confirm the actual stop, not just a tip while the pass is still live.
    for(int down:{1,2,3,4})for(int side:{0,1})for(bool neutral:{false,true})for(int phase:{1,4,5}){
        Case c(down,side,neutral);c.tip();c.stopped();auto out=c.tick(phase);
        bool expected=down>=3&&(side==1||neutral);assert(bool(out.id)==expected);
        if(expected)assert(out.event==live::Event::PassBreakup&&out.side==1-side&&live::cheerType(out)==59&&live::cameraReaction(out.event));
        assert(!c.tick(phase).id);
    }
    // Ordinary incompletions, receiver drops and stale native events remain sound-only.
    for(int kind:{3,4,5}){Case c;c.kind=kind;c.stopped();assert(!c.tick(4).id);}
    {Case c;c.model.reset();c.sample.deflection=10;c.sample.side=0;c.tick(1);c.tick(3);c.stopped();assert(!c.tick(4).id);}
    for(int bad=0;bad<14;++bad){Case c;c.tip();c.stopped();
        if(bad==0)c.allowed=false;if(bad==1)c.p.simulated=true;if(bad==2)c.p.resumed=true;if(bad==3)c.now+=800;
        if(bad==4)++c.epoch;if(bad==5)++c.match;if(bad==6)++c.sequence;if(bad==7)c.result.accepted=false;
        if(bad==8)c.result.firstDown=true;if(bad==9)c.p.interception=true;if(bad==10)c.p.fumble=true;
        if(bad==11)c.result.homePoints=6;if(bad==12)c.kind=1;if(bad==13)c.held=true;
        assert(!c.tick(4).id);
    }
    {Case c;c.sample.side=1;c.sample.deflection=10;c.tick(3);c.stopped();assert(!c.tick(4).id);} // Wrong defender.
    {Case c;c.sample.object=1001;c.sample.deflection=10;c.sample.side=0;c.tick(3);c.stopped();assert(!c.tick(4).id);} // Changed cache is baseline-only.
    {Case c;c.sample.valid=false;c.tick(1);c.tick(3);c.sample={999,true,10,0};c.tick(3);c.stopped();assert(!c.tick(4).id);} // Mid-snap attach.
    {Case c;c.tip();c.tick(4);for(int n=0;n<51;++n)c.tick(4);c.stopped();assert(!c.tick(4).id);} // Late proof expires.
    // One camera owner per play: an earlier knockout, sack or larger stop wins.
    for(auto event:{live::Event::CatchKnockout,live::Event::Sack,live::Event::ForcedFumble,live::Event::CriticalStop}){
        Case c;c.tip();c.tick(3,{event,1,0,0});c.stopped();assert(!c.tick(4).id);
    }
    {Case c;c.tip();c.prior=1;c.stopped();assert(!c.tick(4).id);}
    // Landed hit sticks retain their sound immediately, but only a confirmed
    // third/fourth stop adds motion. A first down or scoring play cannot qualify.
    assert(!live::cameraReaction(live::Event::HitStick)&&live::cheerType({live::Event::HitStick,1,0,0})==59);
    for(int down:{1,2,3,4})for(bool converted:{false,true}){
        Case c(down);assert(!c.tick(3,{live::Event::HitStick,1,0,0}).id);c.stopped();c.result.firstDown=converted;c.held=true;
        auto out=c.tick(4);assert(bool(out.id)==(down>=3&&!converted));if(out.id)assert(out.event==live::Event::HitStickStop);
    }
    {Case c;c.stopped();assert(c.tick(4,{live::Event::HitStick,1,0,0}).event==live::Event::HitStickStop);} // Same-frame hit/result.
    {Case c;c.tip();c.tick(3,{live::Event::HitStick,1,0,0});c.stopped();assert(c.tick(4).event==live::Event::PassBreakup);assert(!c.tick(4).id);}
    {Case c;c.sample.deflection=99;c.sample.side=-1;c.tick(3);assert(c.model.pendingSide()&&c.model.lookupAfter()==0);
        c.sample.side=0;c.tick(3);assert(c.model.deflectionProof()&&!c.model.pendingSide());c.stopped();assert(c.tick(4).event==live::Event::PassBreakup);}
    {Case c;c.sample.deflection=99;c.sample.side=-1;c.tick(3);c.sample.side=1;c.tick(3);c.stopped();assert(!c.tick(4).id);}
    // Small breakup motion: half live strength, 1.8 seconds, with a cheer
    // and a shared roar/swell companion. Native-camera/mute gates stay upstream.
    motion::Input x;x.match=123;x.epoch=1;x.now=1000;x.phase=4;x.allowed=true;x.duringPlay=1;
    live::Outcome event{live::Event::PassBreakup,1,0,0};defense::motion(event,x);
    motion::Model camera;motion::Settings settings;auto cue=camera.observe(settings,x);assert(cue.until==3300&&cue.impactOnly&&cue.percent==6);
    x.now=1200;x.duringPlay=0;cue=camera.observe(settings,x);assert(motion::strength(cue,x.now)>0&&motion::strength(cue,x.now)<=6);
    sustained::Intent sound;sound.active=true;sound.match=123;sound.epoch=1;sound.updated=x.now;
    assert(pairing::merge(sound,cue,x.now,1,1).intensity>0&&live::cheerType(event)==59);
    x.touchdown=1;x.now=1300;assert(camera.observe(settings,x).kind==motion::Kind::Touchdown);
    // A confirmed force has the same ceiling and duration, but reaches it
    // promptly even when taking over an existing pre-snap shake.
    for(bool buildup:{false,true}){
        motion::Model fast,ordinary;motion::Input input;input.match=123;input.epoch=1;input.allowed=true;input.phase=1;input.now=1000;
        input.lineup=input.venueEligible=buildup;input.lineupId=1;
        fast.observe(settings,input);ordinary.observe(settings,input);input.now=2000;fast.observe(settings,input);ordinary.observe(settings,input);
        input.phase=3;input.lineup=false;input.now=2100;fast.observe(settings,input);ordinary.observe(settings,input);
        input.now=2110;input.duringPlay=1;input.duringImpact=true;
        auto normal=ordinary.observe(settings,input);defense::motion({live::Event::ForcedFumble,1,0,0},input);auto forced=fast.observe(settings,input);
        assert(forced.percent==13&&normal.percent==settings.live&&forced.began==normal.began);
        input.now=2145;forced=fast.observe(settings,input);input.forcedImpact=false;normal=ordinary.observe(settings,input);
        assert(motion::strength(forced,input.now)>motion::strength(normal,input.now));
        input.forcedImpact=true;input.now=2160;forced=fast.observe(settings,input);assert(motion::strength(forced,input.now)>=13&&motion::strength(forced,input.now)<=13*1.15f);
        assert(forced.until==5110&&forced.began==2110); // Same ID cannot restart it.
        input.allowed=false;input.now=2170;assert(fast.observe(settings,input).kind==motion::Kind::None);
    }
    std::puts("PASS: captured defender deflection layout; third/fourth confirmed breakup stops; paired sound/motion; ownership and stale guards; fast forced-fumble onset at unchanged peak/duration and smooth takeover; hit-stick sound-only until a confirmed critical-down stop.");
}

