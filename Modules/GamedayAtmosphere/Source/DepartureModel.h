#pragma once
#include "SpringGame.h"
#include "AtmosphereRules.h"
#include "AttendanceContext.h"
#include "LateTraffic.h"

namespace attendance::departures {
inline double strengthFor(const Settings& c){return (std::clamp)(c.formulaStrength,0,200)*0.01;}
inline float pressure(const State& s,float timeLeft){
    return (std::clamp)((std::abs(s.homeScore-s.awayScore)-8)/(8+32*timeLeft),0.0f,1.0f);
}
inline float rankMultiplier(const State& raw){
    State s=ranked(raw);
    if(s.rank0<1||s.rank1<1||s.homeScore==s.awayScore)return 1;
    bool favoriteLeads=(s.homeScore>s.awayScore&&s.rank0<s.rank1)||(s.awayScore>s.homeScore&&s.rank1<s.rank0);
    return favoriteLeads?1+0.20f*context::separation(s):1;
}
inline float stayFactor(const State& s,const History& h){
    if(competitiveOvertime(s))return 0;
    if(h.comeback)return inferiorOpponent(s);
    float stage=(std::clamp)(h.watchStage/3.0f,0.0f,1.0f);
    return 1-0.85f*upsetDiscrepancy(s)*stage;
}
inline bool excellentGame(const State& s,const History& h){
    int margin=std::abs(s.homeScore-s.awayScore);
    return inferiorOpponent(s)<1&&margin<=8&&(marqueeMatchup(s)||h.upsetWatch||h.comeback||
        (margin<=3&&s.homeScore+s.awayScore>=28));
}
inline float homePatience(const State& s,float elapsed){
    bool homeAhead=!s.neutral&&s.homeScore>s.awayScore;
    float home=homeAhead?(elapsed<2*s.quarterSeconds?0.375f:elapsed<3*s.quarterSeconds?0.35f:0.405f):1.0f;
    float inferior=inferiorOpponent(s);
    return home+0.58f*inferior*(1-home)-0.06f*context::comparableHomeWin(s);
}
inline float rainyHomeWinWeight(const State& s,float elapsed){
    // Ease away winning-fan protection late in a decided rainy game, using
    // observed game time rather than a fixed deduction after each score.
    if(!valid(s)||s.neutral||s.playoff||s.quarter!=4||s.phase==7||!badWeather(s)||s.homeScore-s.awayScore<=21)return 0;
    return context::smooth((s.homeScore-s.awayScore-21)/14.f)*
        context::smooth((elapsed-3.5f*s.quarterSeconds)/(.5f*s.quarterSeconds));
}
inline float matchupFactor(const Settings& c,const State& s){
    Profile profile=profileFor(c,s);
    return profile==Profile::Reduced?0.75f+inferiorOpponent(s)*(badWeather(s)?0.5f:0.25f):profile==Profile::BadWeather?1.25f:1.0f;
}
inline int fourthBreakPoints(const Settings& c,const State& s,const History& h){
    if(!c.enabled||!c.formulaStrength||s.quarter<3||s.quarter>4||s.phase==7||stayFactor(s,h)<=0)return 0;
    float margin=static_cast<float>(std::abs(s.homeScore-s.awayScore));
    float severity=context::smooth((margin-8)/20.f);
    float patience=homePatience(s,3*s.quarterSeconds);
    float points=1+6*severity*patience*matchupFactor(c,s)*rankMultiplier(s);
    if(s.playoff)points*=context::smooth((margin-14)/21.f)*(championship(s)?.15f:.35f);
    points*=stayFactor(s,h)*static_cast<float>(strengthFor(c));
    return (std::clamp)(static_cast<int>(std::lround(points)),0,7);
}
inline double homeLossTarget(const Settings& c,const State& s,const History& h,float elapsed){
    // A badly beaten home crowd need not wait for halftime or Q4. This is
    // earned over observed game time, never charged back to the scoring play.
    if(!c.enabled||s.playoff||s.neutral||s.phase==7||s.quarter>3||s.awayScore-s.homeScore<=16)return 0;
    float severity=context::smooth((s.awayScore-s.homeScore-16)/19.0f);
    float progress=(std::clamp)(elapsed/(2*s.quarterSeconds),0.0f,1.0f);
    float loss=(8+27*progress)*.01f*severity*context::smooth(elapsed/(s.quarterSeconds*.75f));
    loss=(std::min)(loss*matchupFactor(c,s)*rankMultiplier(s),context::halftime(s).permanent*.0085f);
    return -std::log(1-loss)*stayFactor(s,h)*strengthFor(c);
}
inline double catchupTarget(const Settings& c,const State& s,const History& h,float elapsed){
    // Only regular-season Q4 blowouts. This is a pressure target, never a
    // direct attendance assignment: already-depleted crowds get no extra cut.
    int margin=std::abs(s.homeScore-s.awayScore);
    if(!c.enabled||s.playoff||s.quarter!=4||s.phase==7||margin<=16||elapsed<3*s.quarterSeconds)return 0;
    double severity=std::pow((std::clamp)((margin-8)/27.0,0.0,1.0),0.65);
    // Rank-based hard floors are enforced by settlement, keeping the pressure
    // itself continuous across No. 25/26 and across a change of leader.
    double loss=(75+(badWeather(s)?10:0))*0.01*severity;
    double progress=(std::clamp)((elapsed-3*s.quarterSeconds)/s.quarterSeconds,0.0f,1.0f);
    // Ease into moderate/major blowouts without a steep 21-to-24-point jump.
    float activation=context::smooth((margin-16)/4.0f)*context::smooth((margin-8)/24.0f);
    double wetWin=1+.65*rainyHomeWinWeight(s,elapsed);
    return -std::log(1-loss)*(0.5+0.5*progress)*homePatience(s,elapsed)*wetWin*matchupFactor(c,s)*
        rankMultiplier(s)*stayFactor(s,h)*activation*context::lateOpportunity(s)*(s.neutral?0.75f:1.0f)*(1+0.05f*context::variation(s))*
        (std::clamp)(c.formulaStrength,0,200)*0.01;
}
inline double nightHomeWinTarget(const Settings& c,const State& s,const History& h,float elapsed){
    // Late-Q3 traffic in a decided night home win, not another loss after
    // each score. Both observed endpoints, existing floors and caps still apply.
    if(!c.enabled||!regularSeason(s)||s.neutral||!s.night||s.phase==7||s.quarter>4||s.homeScore-s.awayScore<21)return 0;
    float severity=.35f+.65f*context::smooth((s.homeScore-s.awayScore-21)/14.f);
    float progress=context::smooth((elapsed-2.5f*s.quarterSeconds)/(1.5f*s.quarterSeconds));
    float loss=(.68f+(badWeather(s)?.04f:0.f))*severity*progress;
    return -std::log(1-loss)*stayFactor(s,h)*strengthFor(c);
}
inline double trafficTarget(const Settings& c,const State& raw,const History& h,float elapsed,float scoredAt){
    if(!c.enabled||!c.formulaStrength||!c.lateTraffic||raw.neutral||raw.quarter!=4||raw.phase==7||raw.homeScore>=raw.awayScore||elapsed<3*raw.quarterSeconds||elapsed>4*raw.quarterSeconds)return 0;
    State s=raw;s.seconds=4*s.quarterSeconds-elapsed;
    float contextScale=(std::clamp)(.5f+.5f*matchupFactor(c,s)*rankMultiplier(s),.85f,1.25f);
    float loss=(std::min)(static_cast<float>(lossCeiling(s)),traffic::lossTarget(s,scoredAt)*contextScale)*.01f;
    return loss>0?-std::log((std::max)(.000001f,1-loss))*stayFactor(s,h)*strengthFor(c):0;
}
inline float rate(const Settings& c,const State& s,const History& h,float elapsed){
    if(!c.enabled||s.quarter>4||s.phase==7)return 0;
    float t=(std::clamp)(1-elapsed/(4*s.quarterSeconds),0.0f,1.0f);
    float p=pressure(s,t),keep=stayFactor(s,h);
    float strength=(std::clamp)(c.formulaStrength,0,200)*0.01f;
    if(s.playoff){
        float gate=(std::clamp)((std::abs(s.homeScore-s.awayScore)-8)/27.0f,0.0f,1.0f);
        float postseasonRate=-std::log(1-lossCeiling(s)*0.01f);
        float home=!s.neutral&&s.homeScore>s.awayScore?0.65f+0.35f*inferiorOpponent(s):1.0f;
        float late=elapsed>=2*s.quarterSeconds?1+context::smooth((std::abs(s.homeScore-s.awayScore)-21)/14.0f):1;
        // Postseason fans stay patient early. Traffic exits build only in a
        // noncompetitive fourth quarter, with championship patience intact.
        float fourth=context::smooth((elapsed-3*s.quarterSeconds)/s.quarterSeconds);
        float decided=context::smooth((std::abs(s.homeScore-s.awayScore)-8)/20.0f);
        late+=fourth*decided*(championship(s)?3.0f:5.0f);
        return postseasonRate*p*p*gate*home*keep*strength*late;
    }
    // Preserve the accepted halftime cut, then soften moderate deficits in Q3
    // and let exits build faster in Q4. Severe blowouts retain their full rate.
    float severity=context::smooth((std::abs(s.homeScore-s.awayScore)-21)/14.0f);
    float late=elapsed>2*s.quarterSeconds+0.001f?(elapsed<3*s.quarterSeconds?0.25f:0.50f):1.0f;
    late+=severity*(1-late);
    // Routine departures remain small in close games; the extra patience
    // applies to blowout-driven exits rather than switching routine losses off.
    float neutral=s.neutral?0.75f:1; // Winning supporters still have a reason to stay.
    float routine=.06f+.02f*context::smooth((std::abs(s.homeScore-s.awayScore)-8)/8.f);
    return (routine+2.4f*p*p*homePatience(s,elapsed)*rankMultiplier(s)*late)*matchupFactor(c,s)*keep*strength*
        context::lateOpportunity(s)*neutral*(1+0.05f*context::variation(s))*context::departureEngagement(s,elapsed);
}
struct Result {int loss=0;bool changed=false,catchup=false,traffic=false;};
// Accrue game-time pressure independently of rendering. settle() is called ONLY
// at existing verified selection/break windows, never by a running clock alone.
struct Model {
    bool seen=false,automatic=false,catchupQueued=false;float elapsed=0,length=0,minimumStart=-1;double hazard=0,committedHazard=0;
    int committedLoss=0,capPeriod=0,periodStartLoss=0;State previous;History priorHistory;
    // Earned departures are independent of how often play selection opens.
    // Rendering follows this bounded target, using its own existing limits.
    int plannedLoss=0,plannedPeriod=0,plannedStart=0;
    int fourthBreakGroup=0;bool fourthBreakNotice=false;
    float lastHomeScoreAt=-1;bool trafficQueued=false;
    int trafficQuarterPolicy=0;
    bool rainyWinQuarterPolicy=false;
    int windowBudget=100;
    float windowAt=-1;int windowHome=-1,windowAway=-1;
    int visibleLow=0,visibleHigh=100,visibleBudget=100;
    bool visibleBound=false;
    int takeFourthBreakNotice(){if(!fourthBreakNotice)return 0;fourthBreakNotice=false;return fourthBreakGroup;}
    void anchorVisible(int loss,int temporary){
        if(visibleBound)return;
        int base=(std::max)(loss,temporary);
        visibleLow=(std::max)(0,base-visibleBudget);visibleHigh=(std::min)(100,base+visibleBudget);visibleBound=true;
    }
    void openingVisible(){visibleBound=false;visibleLow=0;visibleHigh=100;}
    void halftimeVisible(){visibleBound=true;visibleLow=0;visibleHigh=100;} // Main temporary exodus after the half, at the existing safe break only.
    int paceTemporary(int loss,int requested)const{
        int visible=(std::clamp)((std::max)(loss,requested),visibleLow,visibleHigh);
        return visible>loss?visible:0;
    }
    void beginWindow(int limit=100){windowBudget=(std::clamp)(limit,1,100);}
    void beginWindow(const State& s,int limit){
        float time=elapsedGameTime(s);
        if(time==windowAt&&s.homeScore==windowHome&&s.awayScore==windowAway)return;
        // Four points per 30 reference game seconds, with a bounded catch-up
        // allowance for long drives. Quarter scaling works at all lengths.
        float interval=windowAt<0?s.quarterSeconds/24.0f:(std::max)(0.0f,time-windowAt);
        limit=(std::clamp)(limit,1,100);
        int allowance=limit>=100?100:(std::clamp)(static_cast<int>(std::ceil(limit*(std::max)(1.0f,interval/(s.quarterSeconds/24.0f)))),limit,(std::max)(16,limit));
        // Reconcile earned permanent exits at the next hidden opportunity.
        // Otherwise an identical late score path gets different departures
        // solely because one player opens play selection more often.
        if(limit<100)allowance=(std::max)(allowance,(std::max)(0,plannedLoss-committedLoss));
        windowAt=time;windowHome=s.homeScore;windowAway=s.awayScore;beginWindow(allowance);
        visibleBudget=allowance;visibleBound=false;visibleLow=0;visibleHigh=100;
    }
    void reset(){*this=Model{};}
    int quarterLimit(const State& raw,float time,int maximumPerQuarter)const{
        int cap=(std::clamp)(maximumPerQuarter,1,50);
        if(raw.playoff)return (std::min)(cap,championship(raw)?10:20);
        if(raw.quarter!=4||time<3*raw.quarterSeconds||time>4*raw.quarterSeconds)return cap;
        State s=raw;s.seconds=4*s.quarterSeconds-time;
        int wet=rainyWinQuarterPolicy?static_cast<int>(std::lround((std::min)(10,cap/4)*rainyHomeWinWeight(s,time))):0;
        int traffic=traffic::extraQuarterAllowance(s,lastHomeScoreAt,trafficQuarterPolicy);
        // Mutually exclusive home-win/home-loss allowances, never stacked.
        // Custom low quarter caps get a proportionately smaller exception;
        // ranked and postseason crowd floors retain their final authority.
        return cap+(std::max)(wet,traffic);
    }
    void boundPressure(const State& s,float time,int maximumPerQuarter){
        int period=(std::min)(4,1+static_cast<int>(time/s.quarterSeconds+0.00001f));
        if(period!=plannedPeriod){plannedPeriod=period;plannedStart=plannedLoss;}
        int cap=quarterLimit(s,time,maximumPerQuarter);
        int allowed=(std::max)(plannedLoss,(std::min)(lossCeiling(s),plannedStart+cap));
        double ceiling=-std::log((std::max)(0.000001,1-allowed/100.0));
        hazard=(std::min)(hazard,ceiling);
        plannedLoss=(std::max)(plannedLoss,static_cast<int>(std::floor(100*(1-std::exp(-hazard))+0.00001)));
    }
    static float clockWeight(float from,float to,float quarter){
        auto overlap=[&](float start,float end,float budget){
            return budget*(std::max)(0.0f,(std::min)(to,end)-(std::max)(from,start))/(end-start);
        };
        return overlap(quarter,2*quarter,0.10f)+overlap(2*quarter,3*quarter,0.20f)+overlap(3*quarter,4*quarter,0.20f);
    }
    bool observe(const Settings& c,const State& s,const History& h,bool enabled=true,bool authoritative=false){
        if(!valid(s)||(!authoritative&&s.phase!=1&&s.phase!=5&&s.phase!=6))return false;
        float now=elapsedGameTime(s);
        if(spring::verified(s)){
            if(!enabled||!c.enabled||c.formulaStrength<=0)return false;
            if(!seen){length=s.quarterSeconds;seen=true;}
            if(s.quarterSeconds!=length)return false;
            // High-water pressure: drill clock/score resets and replay rewinds
            // cannot restore anyone who has already permanently departed.
            float target=spring::departureTarget(s)*(std::clamp)(c.formulaStrength,0,100)*.01f;
            plannedLoss=(std::max)(plannedLoss,static_cast<int>(std::floor(target)));
            hazard=(std::max)(hazard,-std::log((std::max)(.000001,1-plannedLoss/100.0)));
            elapsed=(std::max)(elapsed,now);previous=s;priorHistory=h;automatic=true;return true;
        }
        if(seen&&(now+0.01f<elapsed||s.homeScore<previous.homeScore||s.awayScore<previous.awayScore||s.quarterSeconds!=length))return false;
        trafficQuarterPolicy=enabled&&c.enabled&&c.formulaStrength>0&&c.lateTraffic&&stayFactor(s,h)>0?(std::clamp)(c.lateTrafficQuarterExtra,0,10):0;
        rainyWinQuarterPolicy=enabled&&c.enabled&&c.formulaStrength>0&&stayFactor(s,h)>0;
        if(!enabled||!c.enabled||!c.formulaStrength||stayFactor(s,h)<=0||s.quarter>4){
            // Protection stops future exits and cancels fractional pressure;
            // already-earned exits do not depend on the next camera window.
            // Manual/disabled time instead adopts the actual displayed crowd.
            bool paused=!enabled||!c.enabled||!c.formulaStrength;
            if(paused){plannedLoss=committedLoss;plannedStart=periodStartLoss;plannedPeriod=capPeriod;}
            double earned=-std::log((std::max)(0.000001,1-plannedLoss/100.0));
            hazard=(std::min)(hazard,paused?committedHazard:earned);
            catchupQueued=trafficQueued=false;
            minimumStart=now<2*s.quarterSeconds?(std::max)(now,s.quarterSeconds):-1;
        }
        if(!seen&&now<2*s.quarterSeconds)minimumStart=(std::max)(now,s.quarterSeconds);
        if(!plannedPeriod){plannedLoss=committedLoss;plannedStart=periodStartLoss;plannedPeriod=capPeriod;boundPressure(s,seen?elapsed:now,c.quarterCap);}
        bool live=enabled&&c.enabled&&s.phase!=6&&s.quarter<=4;
        float previousScoreAt=lastHomeScoreAt;
        if(seen&&automatic&&live){
            if(s.awayScore>previous.awayScore)lastHomeScoreAt=-1;
            else if(s.homeScore-previous.homeScore>=2)lastHomeScoreAt=now;
        }
        if(seen&&live&&(!automatic||stayFactor(previous,priorHistory)<=0))minimumStart=now<2*length?(std::max)(now,length):-1;
        if(seen&&live&&automatic){
            // Integrate in small game-time steps. Both score/history endpoints
            // must permit exits: a new blowout or expired protection is never
            // backdated over an unobserved interval or a Super Sim jump.
            float step=(std::max)(0.10f,length/900.0f);
            for(float a=elapsed;a<now;){
                float b=(std::min)(now,a+step),mid=(a+b)*0.5f;
                float r=(std::min)(rate(c,previous,priorHistory,mid),rate(c,s,h,mid));
                double ordinary=clockWeight(a,b,length)*r;
                hazard+=ordinary*0.5;
                double target=(std::min)((std::max)(catchupTarget(c,previous,priorHistory,mid),homeLossTarget(c,previous,priorHistory,mid)),
                    (std::max)(catchupTarget(c,s,h,mid),homeLossTarget(c,s,h,mid)));
                target=(std::max)(target,(std::min)(nightHomeWinTarget(c,previous,priorHistory,mid),nightHomeWinTarget(c,s,h,mid)));
                double traffic=(std::min)(trafficTarget(c,previous,priorHistory,mid,previousScoreAt),trafficTarget(c,s,h,mid,lastHomeScoreAt));
                bool faster=traffic>target;target=(std::max)(target,traffic);
                // Ordinary catch-up takes about 90 game seconds at 12-minute
                // quarters; hopeless late traffic closes its gap in about
                // 10-15, depending on the deficit and recovery outlook.
                // Endpoint agreement
                // prevents charging a new blowout to earlier competitive time.
                State urgency=s;urgency.seconds=4*length-mid;
                double engagement=(std::min)(context::departureEngagement(previous,mid),context::departureEngagement(s,mid));
                double speed=(faster?48+24*traffic::blowoutUrgency(urgency,lastHomeScoreAt):8)*engagement;
                double extra=(std::max)(0.0,target-hazard)*-std::expm1(-(b-a)/(length/speed));
                hazard+=extra+ordinary*0.5;catchupQueued=catchupQueued||extra>0.000000001;trafficQueued=trafficQueued||(faster&&extra>0.000000001);
                if(mid>=length&&mid<=2*length&&minimumStart>=0){
                    auto halfTarget=[&](const State& x,const History& story){
                        float targetLoss=(context::halftime(x).permanent+0.5f)*0.01f;
                        float progress=context::smooth((mid-minimumStart)/(2*length-minimumStart));
                        float protection=stayFactor(x,story);
                        return -std::log(1-targetLoss*progress*progress)*protection*strengthFor(c);
                    };
                    double wanted=(std::min)(halfTarget(previous,priorHistory),halfTarget(s,h));
                    hazard+=(std::max)(0.0,wanted-hazard)*-std::expm1(-(b-a)/(length/40.0));
                }
                // Clip pressure where it is earned, rather than discarding a
                // different amount at each player's next visual opportunity.
                State bounds=s;bounds.rank0=previous.rank0;bounds.rank1=previous.rank1;
                bounds.homeScore=previous.homeScore;bounds.awayScore=previous.awayScore;
                bounds.precipitation=previous.precipitation;
                boundPressure(bounds,mid,c.quarterCap);
                a=b;
            }
            const float points[]={length,2*length,3*length};
            const float weights[]={0.01f,0.0f};
            for(int i=0;i<2;++i)if(elapsed<points[i]&&now>=points[i])hazard+=weights[i]*rate(c,s,h,points[i]);
            // Replace the old Q4 boundary nudge with one score-based group.
            // Accrue once when observed game time crosses the Q3/Q4 break;
            // settlement still owns the safe camera window, floor and cap.
            if(elapsed<points[2]&&now>=points[2]){
                fourthBreakGroup=fourthBreakPoints(c,s,h);fourthBreakNotice=fourthBreakGroup>0;
                double remaining=(std::max)(.000001,std::exp(-hazard)-fourthBreakGroup*.01);
                hazard=(std::max)(hazard,-std::log(remaining));
            }
            float warning=4*length-120;
            if(c.warningEnabled&&length>120&&elapsed<warning&&now>=warning)hazard+=0.02f*rate(c,s,h,warning);
        }
        boundPressure(s,now,c.quarterCap);
        // Pregame seeds Q1 without banking departures. Late attaches never
        // invent earlier checkpoints; disabled/manual time is not caught up.
        seen=true;elapsed=now;length=s.quarterSeconds;previous=s;priorHistory=h;
        automatic=enabled&&c.enabled&&s.quarter<=4;
        return true;
    }
    Result settle(const State& s,int currentLoss,int extra=0,int maximumPerQuarter=45){
        Result r;r.loss=currentLoss;
        if(!valid(s))return r;
        if(spring::verified(s))extra=0;
        const int before=currentLoss;currentLoss=(std::max)(currentLoss,committedLoss);
        int period=(std::min)(4,1+static_cast<int>(elapsedGameTime(s)/s.quarterSeconds+0.00001f));
        if(period!=capPeriod){capPeriod=period;periodStartLoss=currentLoss;}
        int cap=quarterLimit(s,elapsedGameTime(s),maximumPerQuarter);
        int allowed=(std::min)(lossCeiling(s),periodStartLoss+cap);
        int requested=(std::min)(100,currentLoss+(std::clamp)(extra,0,25));
        // Fold verified external late-score exits into the same history once.
        double retained=(std::max)(0.000001,1-requested/100.0);
        double anchor=-std::log(retained);
        if(requested>committedLoss)anchor+=(std::max)(0.0,hazard-committedHazard);
        hazard=(std::max)(hazard,anchor);
        if(requested>plannedLoss){plannedLoss=requested;plannedStart=(std::max)(plannedStart,periodStartLoss);}
        boundPressure(s,elapsedGameTime(s),maximumPerQuarter);
        int loss=static_cast<int>(std::floor(100*(1-std::exp(-hazard))+0.00001));
        int bounded=(std::max)(currentLoss,(std::clamp)(loss,0,allowed));
        r.loss=(std::min)(bounded,currentLoss+windowBudget);
        windowBudget-=(std::max)(0,r.loss-currentLoss);
        // The model already discarded pressure blocked by its floors/caps.
        // A delayed visual update must not rewrite the accumulated pressure.
        committedHazard=-std::log((std::max)(0.000001,1-r.loss/100.0));committedLoss=r.loss;
        r.changed=r.loss!=before;r.catchup=r.changed&&catchupQueued;r.traffic=r.changed&&trafficQueued;catchupQueued=trafficQueued=false;return r;
    }
    Result convertSeatBreakers(const State& s,int current,int points,int cap){
        // These fans are already offscreen/absent. Reclassification consumes
        // the ordinary quarter/floor allowance without another visible drop.
        int saved=windowBudget;windowBudget=(std::clamp)(points,0,25);
        auto result=settle(s,current,points,cap);windowBudget=saved;return result;
    }
};
inline const char* reason(const Settings& c,const State& s,const History& h,bool catchup=false,bool traffic=false){
    if(spring::verified(s))return s.quarter<=2?"spring exhibition; modest score-independent first-half departures":"spring exhibition; permanent halftime wave and score-independent gradual thinning";
    if(h.comeback&&inferiorOpponent(s)<1)return "comeback protection";
    if(competitiveOvertime(s))return "competitive overtime";
    if(h.watchStage>0)return "upset interest is slowing departures";
    if(traffic)return "late traffic; deficit, clock, possession, timeouts and recent scoring";
    if(s.playoff)return s.quarter==4&&std::abs(s.homeScore-s.awayScore)>8?"late postseason traffic departures":"patient postseason crowd";
    if(catchup&&rainyHomeWinWeight(s,elapsedGameTime(s))>0)return "late rainy home blowout; winning-fan protection reduced gradually";
    if(catchup)return s.quarter<4?"home blowout; earlier departures catching up to the score":"late blowout; departures catching up to the score";
    if(std::abs(s.homeScore-s.awayScore)<=8)return "competitive game; routine departures only";
    if(profileFor(c,s)==Profile::BadWeather)return "score, time remaining and bad weather";
    if(!s.neutral&&s.homeScore>s.awayScore&&inferiorOpponent(s)<1)return "home lead is keeping more fans around";
    return "score, time remaining and matchup";
}
}
