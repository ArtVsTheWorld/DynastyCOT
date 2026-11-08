#include "../Source/ExcitementRules.h"
#include "../Source/CrowdAudioModel.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace attendance;
using namespace attendance::energy;
static bool near(float a,float b){return std::fabs(a-b)<0.001f;}
static Play game(){Play p;p.game={1,0,0,1,300,300};p.down=1;p.offense=0;p.toGoal=50;p.toFirst=10;return p;}
int main(){
    energy::Settings c;c.opening=0;
    // The promised table: full original attendance, close game, before event energy.
    for(int kind=0;kind<5;++kind){
        Play p=game();
        if(kind==1)p.game.rivalry=true;
        if(kind==2){p.game.rivalry=true;p.game.rank0=6;p.game.rank1=11;}
        if(kind>=3){p.game.playoff=true;p.game.bowl=kind==4?47:40;}
        Model m;m.update(c,p,0,1);
        const float table[]={0,0,3,3,4};float expected=table[kind];
        assert(near(m.gainDb(c),expected));
        assert(near(m.ambienceLift,kind<2?0.0f:6.0f));
        float bed=mix::ambienceForTotal(m.gainDb(c),m.gainDb(c),m.ambienceLift,c.maxDb);
        assert(near(m.gainDb(c)+bed,kind<2?0.0f:6.0f));
        // Neutral labels receive identical boosts.
        p.game.neutral=true;Model neutral;neutral.update(c,p,0,1);
        assert(near(neutral.gainDb(c),m.gainDb(c))&&near(neutral.ambienceLift,m.ambienceLift));
        if(kind>=2){
            p.game.quarter=2;Model middle;middle.update(c,p,0,1);assert(middle.gainDb(c)<m.gainDb(c));
            p.game.quarter=1;Model sparse;sparse.update(c,p,0,0.25f);
            assert(sparse.gainDb(c)<m.gainDb(c)&&sparse.ambienceLift<m.ambienceLift);
            p.game.awayScore=42;Model blowout;blowout.update(c,p,0,1);
            assert(blowout.gainDb(c)==0&&blowout.ambienceLift==0);
        }
    }
    // Child/parent headroom remains valid during both rising and falling gains.
    for(int requested=0;requested<=48;++requested)for(int applied=0;applied<=32;++applied)for(int extra=0;extra<=12;++extra){
        float parent=requested*0.25f,old=applied*0.25f;
        float bed=mix::ambienceGain(parent,old,extra*0.25f,8);
        float safe=mix::parentGain(parent,bed,8);
        assert(bed>=0&&safe>=0&&bed+safe<=8.001f&&bed+old<=8.001f);
    }
    assert(mix::ambienceGain(8,7,1,8)==0);
    assert(mix::parentGain(8,2,8)==6); // A retained child layer reserves its headroom.
    assert(mix::ambienceGain(NAN,3,1,8)==0&&mix::parentGain(NAN,1,8)==0);
    // A user/custom value cannot escape the new hard cap.
    c.maxDb=16;c.bigGameDb=c.rankedRivalryDb=c.playoffDb=c.championshipDb=16;
    Play loud=game();loud.game.playoff=true;loud.game.bowl=47;Model cap;
    cap.update(c,loud,0,1);cap.event(Event::OpeningTouchdown,0,loud.game);
    for(int i=0;i<400;++i){cap.update(c,loud,i*0.05,1);assert(cap.gainDb(c)<=6);}
    c=energy::Settings{};
    // Scoring pace rewards competitive shootouts, without making defensive games quieter.
    State shootout{3,35,38,1,150,300};ScoringPace pace;pace.advance(shootout);assert(pace.value(shootout)>5&&pace.value(shootout)<=10);
    State defensive=shootout;defensive.homeScore=7;defensive.awayScore=10;ScoringPace tense;tense.advance(defensive);assert(tense.value(defensive)==0);
    State rout=shootout;rout.homeScore=7;rout.awayScore=49;ScoringPace dull;dull.advance(rout);assert(dull.value(rout)==0);
    State opening{1,7,0,1,290,300};ScoringPace early;early.scored(opening,6);assert(early.value(opening)<0.1f);
    // Equivalent progress on different quarter lengths produces the same pace.
    State longGame=shootout;longGame.quarterSeconds=900;longGame.seconds=450;ScoringPace scaled;scaled.advance(longGame);assert(near(pace.value(shootout),scaled.value(longGame)));
    pace.scored(shootout,6);float hot=pace.value(shootout);assert(hot<=10);
    shootout.seconds-=100;pace.advance(shootout);assert(pace.value(shootout)<hot);
    float recent=pace.recent;shootout.seconds+=50;pace.advance(shootout);assert(near(pace.recent,recent));
    // Actual completed scores add recent heat once; disabled mode contributes none.
    Model scoring;Outcome result;result.accepted=result.completed=true;result.homePoints=6;result.sequence=1;
    scoring.react(c,result,0,shootout);scoring.update(c,Play{shootout},0,1);assert(scoring.scoringLift>0);
    c.highScoring=0;scoring.update(c,Play{shootout},1,1);assert(scoring.scoringLift==0);c.highScoring=1;
    // Same-match short gaps hold a verified target, then release instead of sticking forever.
    mix::Continuity continuity;assert(continuity.record(1,0,8,8)==8);
    assert(continuity.gap(1,1,true,8)==8&&near(continuity.gap(1,2,true,8),4));
    assert(continuity.gap(1,3,true,8)==0);
    continuity.record(1,4,8,8);assert(continuity.gap(2,4.1,true,8)==0);
    continuity.record(1,5,8,8);assert(continuity.gap(1,5.1,false,8)==0);
    // Reproduce the winning OT field goal -> Quarter_EndOfGame transition.
    Tracker tracker;Model winner;Play p=game();p.game={5,20,20,1,0,300};
    for(int i=0;i<200;++i){tracker.observe(c,p);winner.update(c,p,i*0.05,1);}
    p.game.phase=3;tracker.observe(c,p);winner.update(c,p,10,1);float outgoing=winner.gainDb(c);
    p.game.phase=7;p.game.quarter=6;p.game.homeScore=23;p.game.seconds=300;
    auto win=tracker.observe(c,p);assert(win.event==Event::OvertimeWin);
    assert(winner.react(c,win,10.05,p.game));winner.update(c,p,10.05,1);
    assert(winner.gainDb(c)>=outgoing&&winner.gainDb(c)<=8);
    for(int i=1;i<=80;++i){
        p.game.seconds=i%2?0.0f:300.0f;auto same=tracker.observe(c,p);assert(same.accepted&&same.event==Event::None);
        winner.update(c,p,10.05+i*0.05,1);assert(winner.gainDb(c)>=outgoing-0.001f&&winner.gainDb(c)<=8);
    }
    float previous=winner.gainDb(c);
    for(int i=81;i<400;++i){winner.update(c,p,10.05+i*0.05,1);float db=winner.gainDb(c);assert(db<=8&&previous-db<0.2f);previous=db;}
    winner.reset();assert(winner.victoryAt<0&&winner.victoryDb==0);
    // Gentler attendance reduction: 20 softens the old 35 curve without compounding.
    audio::Gain soft,old;soft.attendance(0.2f,0.2f);old.attendance(0.2f,0.35f);
    soft.advance(2);old.advance(2);assert(soft.value()>old.value()&&soft.value()<1);
    std::cout<<"PASS: matchup/ambience table, steady-bed priority and 6 dB normal cap, competitive scoring pace, bounded continuity, OT winner handoff and gentler attendance audio.\n";
}
