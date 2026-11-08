#include "../Source/DepartureModel.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace attendance;
struct Run {int q2=0,half=0,q4=0,warning=0,final=0,breakGroup=0;double hazard=0;};
Run simulate(State s,int step=5,Settings c={}){
    departures::Model model;History h;Run r;int loss=0;
    int q=static_cast<int>(s.quarterSeconds);
    for(int quarter=1;quarter<=4;++quarter){
        s.quarter=quarter;s.phase=5;
        for(int seconds=q;seconds>=0;seconds=(std::max)(-1,seconds-step)){
            s.seconds=static_cast<float>(seconds);assert(h.observe(c,s));assert(model.observe(c,s,h));
            int before=loss;loss=model.settle(s,loss,0,c.quarterCap).loss;assert(loss>=before&&loss<=lossCeiling(s));
            if(seconds==q){if(quarter==2)r.q2=loss;if(quarter==3)r.half=loss;if(quarter==4)r.q4=loss;}
            if(quarter==4&&seconds==120)r.warning=loss;
        }
    }
    r.final=loss;r.hazard=model.hazard;r.breakGroup=model.fourthBreakGroup;return r;
}
Run lateBlowout(State s,bool homeWins,int step=5){
    Settings c;History h;departures::Model model;Run r;int loss=0;
    int q=static_cast<int>(s.quarterSeconds);
    for(int time=0;time<=4*q;time+=step){
        s.quarter=(std::min)(4,time/q+1);s.seconds=static_cast<float>(s.quarter*q-time);s.phase=5;
        int added=time<=3*q?0:(std::min)(28,7*static_cast<int>((time-3*q)/(q/8.0)));
        s.homeScore=14+(homeWins?added:0);s.awayScore=14+(homeWins?0:added);
        assert(h.observe(c,s));assert(model.observe(c,s,h));
        int before=loss;loss=model.settle(s,loss,0,c.quarterCap).loss;
        assert(loss>=before&&loss<=lossCeiling(s));
        if(time==3*q)r.q4=loss;
    }
    r.final=loss;r.hazard=model.hazard;return r;
}
struct CadenceRun {int loss=0,planned=0;double hazard=0;};
CadenceRun variedWindows(int minutes,int interval,bool rally,bool postseason=false){
    Settings c;History h;departures::Model d;State s{1,0,0,1,minutes*60.0f,minutes*60.0f};int loss=0;
    s.rank0=50;s.rank1=60;s.playoff=postseason;s.bowl=37;
    for(int tick=0;tick<=2880;++tick){
        float time=tick*s.quarterSeconds/720;
        s.quarter=(std::min)(4,tick/720+1);s.seconds=s.quarter*s.quarterSeconds-time;s.phase=1;
        if(rally){
            s.awayScore=tick<360?7:tick<720?21:tick<1000?28:tick<2600?28:42;
            s.homeScore=tick<1500?0:tick<1800?7:tick<2100?14:21;
        }else{
            s.homeScore=14;s.awayScore=tick<2736?14:tick<2780?21:tick<2823?28:tick<2866?35:42;
        }
        assert(h.observe(c,s)&&d.observe(c,s,h));
        if(tick%interval==0||tick%720==0||tick==2880){
            s.phase=5;d.beginWindow(s,c.transitionCap);int before=loss;
            loss=d.settle(s,loss,0,c.quarterCap).loss;
            assert(loss>=before&&loss<=lossCeiling(s)&&loss-d.periodStartLoss<=c.quarterCap+(s.quarter==4&&!s.playoff?10:0));
        }
    }
    return {loss,d.plannedLoss,d.hazard};
}
int main(){
    // Identical score/possession observations with different renderer timing.
    // Common safe endpoints reconcile earned exits; no camera window can earn
    // pressure, and the existing integer quarter caps still take precedence.
    for(int minutes:{1,6,12,20})for(bool rally:{false,true})for(bool post:{false,true}){
        auto a=variedWindows(minutes,15,rally,post),b=variedWindows(minutes,60,rally,post),d=variedWindows(minutes,120,rally,post);
        assert(a.planned==b.planned&&a.planned==d.planned);
        assert(std::abs(a.hazard-b.hazard)<0.000001&&std::abs(a.hazard-d.hazard)<0.000001);
        assert((std::max)({a.loss,b.loss,d.loss})-(std::min)({a.loss,b.loss,d.loss})<=1);
    }
    Settings c;State s{1,0,0,5,900,900};History h;departures::Model m;
    // An independent analytical oracle for constant routine pressure.
    Run close=simulate(s);assert(close.hazard>.05&&close.hazard<.10&&close.final>=5&&close.final<=8);
    assert(close.q2==0&&close.half>=3&&close.half<=5);
    s.awayScore=70;Run blowout=simulate(s);
    assert(blowout.final==75);
    assert(blowout.q2>=10&&blowout.half>blowout.q2+10&&blowout.q4>blowout.half);
    s.precipitation=1;assert(simulate(s).final<=85&&simulate(s).final>=70);
    for(int kind:{1,2})for(int weather:{0,1,2}){
        s.playoff=true;s.bowl=kind==2?47:37;s.precipitation=weather;
        auto r=simulate(s);assert(r.final<=lossCeiling(s)&&r.final>=lossCeiling(s)-12);
        s.awayScore=21;auto moderate=simulate(s);
        assert(moderate.half<=5&&moderate.final>moderate.q4&&moderate.final<(kind==1?35:20));
        s.awayScore=70;
    }
    s={1,0,21,5,720,720};Run ordinary=simulate(s);
    assert(ordinary.final>=35&&ordinary.final<=60&&ordinary.half>=10&&ordinary.half<=20);
    // The new break group belongs to the Q4 boundary, not ordinary Q3 drift.
    int thirdQuarterDrift=ordinary.q4-ordinary.half-ordinary.breakGroup;
    assert(thirdQuarterDrift<10&&ordinary.final-ordinary.q4+ordinary.breakGroup>thirdQuarterDrift);
    State gap=s;int last=ordinary.final;
    for(int margin=22;margin<=28;++margin){gap.awayScore=margin;int loss=simulate(gap).final;assert(loss>=last&&loss-last<=5);last=loss;}
    gap.awayScore=24;assert(simulate(gap).final-ordinary.final<=10);
    // Traffic pressure starts in Q4 and builds smoothly. Close games and OT
    // are excluded; home winners and national-title crowds remain more patient.
    History traffic;State post{4,0,21,5,720,720};post.playoff=true;post.bowl=37;
    float start=departures::rate(Settings{},post,traffic,2160),middle=departures::rate(Settings{},post,traffic,2520),late=departures::rate(Settings{},post,traffic,2820);
    assert(middle>start&&late>middle);
    State title=post;title.bowl=47;assert(departures::rate(Settings{},title,traffic,2820)<late);
    post.homeScore=21;post.awayScore=0;assert(departures::rate(Settings{},post,traffic,2820)<late);
    post.homeScore=14;post.awayScore=21;assert(departures::rate(Settings{},post,traffic,2820)==0);
    post.quarter=5;assert(departures::rate(Settings{},post,traffic,2820)==0);
    State homeWin=s;homeWin.homeScore=35;homeWin.awayScore=0;Run winning=simulate(homeWin);
    assert(winning.final<=65&&winning.q4-winning.half<=15);
    homeWin.rank0=1;homeWin.rank1=127;assert(simulate(homeWin).final>=winning.final);
    s.homeScore=21;s.awayScore=0;assert(simulate(s).final<ordinary.final);
    s=reversed(s);s.rivalry=true;Run rival=simulate(s);assert(rival.final==ordinary.final);
    s.precipitation=1;assert(profileFor(c,s)==Profile::BadWeather&&simulate(s).final>=rival.final); // Weather comfort/floor can differ; the reduced preset keeps priority.
    s.rivalry=false;assert(simulate(s).final>ordinary.final);
    s.precipitation=0;s.rank0=50;s.rank1=1;Run mismatch=simulate(s);
    float mismatchRate=departures::rankMultiplier(s);s.rank0=127;
    assert(simulate(s).final>=mismatch.final&&departures::rankMultiplier(s)>mismatchRate); // Whole crowd points can round alike.
    float previous=1;
    for(int rank=2;rank<=255;++rank){s.rank0=rank;float next=departures::rankMultiplier(s);assert(next>previous);previous=next;}
    s.rank0=0;s.team0=1500;assert(departures::rankMultiplier(s)>previous);
    s.team0=-1;assert(departures::rankMultiplier(s)==1); // Unknown FBS is not made up.
    // Neutral sites follow either team's story identically.
    s={1,0,21,5,900,900};s.neutral=true;s.rank0=127;s.rank1=1;
    assert(simulate(s).final==simulate(reversed(s)).final);
    s.homeScore=21;s.awayScore=14;h.observe(c,s);h.watchStage=3;
    float massive=departures::stayFactor(s,h);s.rank0=26;assert(departures::stayFactor(s,h)>massive);
    h.comeback=true;assert(departures::rate(c,s,h,3000)==0);
    // No historical departures on late attach; pauses and score/clock rewinds
    // neither consume opportunities nor poison the accepted high-water state.
    m.reset();h.reset();s={3,0,35,5,450,900};assert(m.observe(c,s,h));assert(m.settle(s,0).loss==0);
    for(int i=0;i<500;++i)assert(m.observe(c,s,h));assert(m.hazard==0);
    s.seconds=460;assert(!m.observe(c,s,h));s.seconds=440;s.awayScore=28;assert(!m.observe(c,s,h));
    s.awayScore=35;assert(m.observe(c,s,h)&&m.hazard>0);
    double once=m.hazard;assert(m.observe(c,s,h)&&m.hazard==once);
    // A newly developed blowout cannot charge its higher rate to earlier time.
    m.reset();s={2,0,0,5,900,900};m.observe(c,s,h);s.awayScore=35;s.seconds=450;m.observe(c,s,h);
    assert(m.hazard>0&&m.hazard<0.04&&!m.catchupQueued); // Only the earlier close game's ordinary/halftime pressure.
    double before=m.hazard;s.seconds=400;m.observe(c,s,h);assert(m.hazard>before+0.008);
    // Protection and disabling do not bank exits for a later catch-up.
    for(bool manual:{false,true}){
        m.reset();s={2,0,35,5,900,900};m.observe(c,s,h);
        h.comeback=!manual;s.seconds=600;m.observe(c,s,h,!manual);
        h.comeback=false;s.seconds=300;m.observe(c,s,h);assert(m.hazard==0);
        s.seconds=200;m.observe(c,s,h);assert(m.hazard>0);
    }
    m.reset();s={2,0,35,5,900,900};m.observe(c,s,h);
    s.seconds=450;m.observe(c,s,h);assert(m.hazard>0);
    int earned=m.plannedLoss;assert(earned>0);h.comeback=true;m.observe(c,s,h);
    assert(m.settle(s,0).loss==earned);double protectedHazard=m.hazard;
    s.seconds=300;m.observe(c,s,h);h.comeback=false;s.seconds=250;m.observe(c,s,h);assert(m.hazard==protectedHazard);
    // External late-score losses join the accumulated history without refill.
    m.reset();s={4,0,35,5,300,900};m.observe(c,s,h);assert(m.settle(s,40).loss==40);
    assert(m.settle(s,0).loss==40);s.homeScore=35;s.seconds=290;m.observe(c,s,h);assert(m.settle(s,40).loss>=40);
    m.reset();s={4,0,35,5,900,900};m.observe(c,s,h);m.settle(s,40);
    s.seconds=600;m.observe(c,s,h);double queued=m.hazard-m.committedHazard;assert(queued>0);
    int combined=m.settle(s,45).loss;assert(combined>45);
    assert(m.settle(s,combined).loss==combined); // External + ordinary exits applied once.
    m.reset();s.precipitation=1;assert(m.settle(s,90).loss==90);
    s.precipitation=0;assert(m.settle(s,80).loss==90); // A higher floor cannot refill earlier exits.
    s.quarter=5;s.homeScore=s.awayScore=35;s.seconds=0;m.observe(c,s,h);before=m.hazard;
    s.seconds=900;m.observe(c,s,h);assert(m.hazard==before&&m.settle(s,40).loss>=40);
    // Quarter and warning charges are independent of callback frequency.
    s={1,0,21,5,900,900};auto fine=simulate(s,1),coarse=simulate(s,30);
    assert(std::abs(fine.hazard-coarse.hazard)<0.0001&&fine.final==coarse.final);
    for(int minutes=1;minutes<=20;++minutes){
        s.quarterSeconds=static_cast<float>(minutes*60);s.seconds=s.quarterSeconds;s.awayScore=0;
        auto r=simulate(s,1);assert(std::isfinite(r.hazard)&&r.half>=3&&r.half<=5&&r.final>=5&&r.final<=8);
        s.awayScore=21;r=simulate(s,1);
        assert(r.half>=10&&r.half<=20&&r.final>=35&&r.final<=60);
        assert(r.q4-r.half-r.breakGroup<10&&r.final-r.q4+r.breakGroup>r.q4-r.half-r.breakGroup);
        s.homeScore=35;s.awayScore=0;r=simulate(s,1);assert(r.final<=65);
        s.homeScore=0;
    }
    c.warningEnabled=0;s={1,0,0,5,900,900};assert(simulate(s,5,c).half>=3);
    s={1,17,17,5,720,720};assert(simulate(s).half<10);
    s={1,0,0,5,720,720};s.rivalry=true;assert(simulate(s).half>=3);
    s.rivalry=false;s.rank0=6;s.rank1=11;assert(simulate(s).half<10);
    // Cap cumulative original-crowd percentage points, including late-score extras.
    m.reset();h.reset();s={2,0,70,5,720,720};c.quarterCap=30;m.observe(c,s,h);int capped=0;
    for(int clock=715;clock>0;clock-=5){s.seconds=static_cast<float>(clock);m.observe(c,s,h);capped=m.settle(s,capped,0,30).loss;assert(capped<=30);}
    auto extra=m.settle(s,capped,5,30);assert(extra.loss<=30);
    s.quarter=3;s.seconds=720;m.observe(c,s,h);int halfLoss=m.settle(s,capped,0,30).loss;assert(halfLoss-capped<=30);
    s.seconds=715;m.observe(c,s,h);assert(m.settle(s,halfLoss).loss-halfLoss<=1); // Blocked debt is discarded.
    c.quarterCap=45;
    State inferior{3,35,0,5,720,720};inferior.rank0=1;inferior.rank1=127;
    State equal=inferior;equal.rank1=2;assert(inferiorOpponent(inferior)==1&&inferiorOpponent(equal)==0);
    assert(departures::rate(c,inferior,h,1800)>departures::rate(c,equal,h,1800));
    inferior.rank1=0;inferior.team1=1500;assert(inferiorOpponent(inferior)==1);
    assert(c.quarterCap==45);
    for(int rank:{1,25})for(int weather:{0,1,2})for(bool fcs:{false,true}){
        State winner{1,70,0,5,720,720};winner.rank0=rank;winner.rank1=fcs?0:127;winner.team1=fcs?1500:1158;winner.precipitation=weather;
        assert(simulate(winner).final==65); // Severe mismatch still retains 35%.
        m.reset();h.reset();winner.quarter=4;winner.seconds=100;winner.homeScore=0;winner.awayScore=35;
        m.observe(c,winner,h);assert(m.settle(winner,70).loss==70);
        winner.homeScore=42;winner.awayScore=35;winner.seconds=90;
        h.observe(c,winner);m.observe(c,winner,h);assert(m.settle(winner,70).loss==70); // No refill after taking the lead.
        m.reset();winner.homeScore=70;winner.awayScore=0;m.observe(c,winner,h);m.hazard=10;
        assert(m.settle(winner,65,25).loss==65); // Late-score extras share the hard floor.
    }
    for(int kind:{0,1,2}){
        m.reset();s={2,0,70,5,720,720};s.playoff=kind!=0;s.bowl=kind==2?47:37;
        m.observe(c,s,h);m.hazard=10;int limit=kind==0?45:kind==1?20:10;
        auto cap=m.settle(s,0,5);assert(cap.loss==limit&&m.settle(s,cap.loss,5).loss==limit);
    }
    // Q4 catch-up targets only a confirmed 21+ regular-season margin. It
    // shares the existing home/rank/weather/protection multipliers and floors.
    h.reset();s={4,14,42,5,360,720};s.rank0=26;s.rank1=50;
    double target=departures::catchupTarget(c,s,h,2520);assert(target>0);
    State win=s;win.homeScore=42;win.awayScore=14;
    assert(departures::catchupTarget(c,win,h,2520)<target*0.5);
    double homeTarget=departures::catchupTarget(c,win,h,2520);
    win.team1=1500;assert(departures::catchupTarget(c,win,h,2520)>homeTarget&&departures::catchupTarget(c,win,h,2520)<target);
    State context=s;context.rivalry=true;assert(bigGameMatchup(context)&&departures::catchupTarget(c,context,h,2520)<target&&departures::catchupTarget(c,context,h,2520)>0);
    double rivalTarget=departures::catchupTarget(c,context,h,2520);
    float rivalFactor=departures::matchupFactor(c,context);
    context.precipitation=1;assert(profileFor(c,context)==Profile::Reduced&&departures::matchupFactor(c,context)==rivalFactor);
    assert(departures::catchupTarget(c,context,h,2520)>rivalTarget); // Weather pressure remains in a marquee game.
    context.rivalry=false;assert(departures::catchupTarget(c,context,h,2520)>target);
    context=s;context.rank0=127;assert(departures::catchupTarget(c,context,h,2520)>target);
    context=s;context.neutral=true;context.rank0=127;context.rank1=1;
    assert(departures::catchupTarget(c,context,h,2520)==departures::catchupTarget(c,reversed(context),h,2520));
    for(int margin:{0,8,14,16}){context=s;context.awayScore=14+margin;assert(departures::catchupTarget(c,context,h,2520)==0);}
    context=s;context.quarter=3;assert(departures::catchupTarget(c,context,h,1800)==0);
    context=s;context.phase=7;assert(departures::catchupTarget(c,context,h,2520)==0);
    context=s;context.quarter=5;assert(departures::catchupTarget(c,context,h,2880)==0);
    for(int bowl:{37,47}){context=s;context.playoff=true;context.bowl=bowl;assert(departures::catchupTarget(c,context,h,2520)==0);}
    h.comeback=true;assert(departures::catchupTarget(c,s,h,2520)==0);h.reset();
    // No score-tick cut, no charge for the earlier competitive time in a Sim
    // jump, no pause accumulation, and no duplicate settlement.
    m.reset();s={4,14,14,5,720,720};m.observe(c,s,h);m.settle(s,0);
    s.seconds=420;s.awayScore=49;m.observe(c,s,h);
    // Scoring-pace patience may slightly reduce this routine interval; the
    // new blowout still cannot be charged to the earlier competitive time.
    assert(m.hazard>=0.06*0.20*300/720*.975-0.000001&&m.hazard<=0.06*0.20*300/720*1.0375+0.000001&&!m.catchupQueued);
    double unchanged=m.hazard;m.observe(c,s,h);assert(m.hazard==unchanged);
    s.seconds=330;m.observe(c,s,h);assert(m.catchupQueued&&m.hazard>unchanged+0.2);
    auto caught=m.settle(s,0);assert(caught.changed&&caught.catchup);
    assert(!m.settle(s,caught.loss).catchup&&m.settle(s,caught.loss).loss==caught.loss);
    s.seconds=320;m.observe(c,s,h);assert(m.catchupQueued);
    h.comeback=true;m.observe(c,s,h);assert(!m.catchupQueued&&m.hazard==m.committedHazard);
    h.reset();s.seconds=310;m.observe(c,s,h);assert(!m.catchupQueued); // No backdating the protection's end.
    s.seconds=300;m.observe(c,s,h);assert(m.catchupQueued);
    m.observe(c,s,h,false);assert(!m.catchupQueued&&m.hazard==m.committedHazard);
    // An already-depleted crowd is not charged a second catch-up penalty.
    m.reset();m.observe(c,s,h);m.settle(s,70);s.seconds=290;m.observe(c,s,h);
    assert(!m.catchupQueued&&m.settle(s,70).loss>=70);
    // Late attach seeds only; the rule starts with subsequent live progress.
    m.reset();m.observe(c,s,h);assert(m.hazard==0&&m.settle(s,0).loss==0);
    // Compare complete score paths and confirm quarter-length normalization.
    s={1,0,0,5,720,720};s.rank0=26;s.rank1=50;
    auto lateLoss=lateBlowout(s,false),lateWin=lateBlowout(s,true);
    assert(lateLoss.final>=45&&lateLoss.final<=lateLoss.q4+c.quarterCap+10);
    assert(lateWin.final+10<lateLoss.final);
    context=s;context.rivalry=true;auto lateRival=lateBlowout(context,false);
    // Close marquee rivalries now retain more fans at halftime; once the
    // fourth-quarter rout develops the shared reduced profile remains modest, with positive blowout pressure.
    assert(lateRival.q4<lateLoss.q4&&lateRival.final<=lateLoss.final);
    context.quarter=4;context.homeScore=14;context.awayScore=42;context.seconds=90;
    State ordinaryRout=context;ordinaryRout.rivalry=false;
    assert(std::abs(departures::rate(c,context,h,2790)/departures::rate(c,ordinaryRout,h,2790)-.75)<.000001);
    auto frequent=lateBlowout(s,false,1),infrequent=lateBlowout(s,false,30);
    // Coarser score observations deliberately withhold newly discovered
    // pressure over the uncertain interval. They may be slightly more patient.
    assert(frequent.final>=infrequent.final&&frequent.final<=infrequent.final+1);
    auto constantQ4=[&](int interval){
        departures::Model model;History story;State x{4,0,21,5,720,720};int loss=10;
        model.observe(c,x,story);model.settle(x,loss);
        for(int time=interval;time<=720;time+=interval){x.seconds=static_cast<float>(720-time);model.observe(c,x,story);loss=model.settle(x,loss).loss;}
        Run r;r.final=loss;r.hazard=model.hazard;return r;
    };
    auto steadyFine=constantQ4(1),steadyCoarse=constantQ4(30);
    assert(steadyFine.final==steadyCoarse.final&&std::abs(steadyFine.hazard-steadyCoarse.hazard)<0.0001);
    for(int minutes=1;minutes<=20;++minutes){
        context=s;context.quarterSeconds=static_cast<float>(minutes*60);
        auto r=lateBlowout(context,false,1);assert(r.final>=40&&r.final-r.q4<=c.quarterCap+10);
        assert(lateBlowout(context,true,1).final+8<r.final);
    }
    // Early home routs earn exits before halftime without backdating a new score.
    h.reset();State early{2,0,27,5,637,720};early.rank0=80;early.rank1=24;
    assert(departures::homeLossTarget(c,early,h,803)>0.1);
    State protectedGame=early;protectedGame.playoff=true;assert(departures::homeLossTarget(c,protectedGame,h,803)==0);
    protectedGame=early;protectedGame.neutral=true;assert(departures::homeLossTarget(c,protectedGame,h,803)==0);
    protectedGame=early;std::swap(protectedGame.homeScore,protectedGame.awayScore);assert(departures::homeLossTarget(c,protectedGame,h,803)==0);
    protectedGame=early;protectedGame.awayScore=16;assert(departures::homeLossTarget(c,protectedGame,h,803)==0);
    m.reset();m.observe(c,early,h);assert(m.hazard==0);early.seconds-=90;m.observe(c,early,h);assert(m.plannedLoss>=7&&m.plannedLoss<=45);
    c.enabled=0;assert(simulate(s,5,c).final==0);
    std::cout<<"PASS: analytic formula, Q4 late-blowout catch-up, distributed exits, score/time/home/rank/weather context, FCS ordering, all floors, caps, postseason gating, replay/late attach, protection, OT, no refill and callback invariance.\n";
}
