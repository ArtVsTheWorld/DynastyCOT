#include "../Source/ExcitementRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace attendance;
using namespace attendance::energy;
static Play opening(){Play p;p.game={1,0,0,6,300,300};p.game.rivalry=true;p.game.rank0=12;p.game.rank1=70;p.offense=1;p.down=0;p.toGoal=40;p.toFirst=10;return p;}
static bool near(float a,float b){return std::fabs(a-b)<0.001f;}
int main(){
    energy::Settings c;
    for(int side:{0,1}){
        Play p=opening();p.offense=side;p.game.neutral=true;
        Model model;model.update(c,p,0,0.6f,1);
        assert(model.openingWeight==1&&near(model.ambienceLift,6)&&model.gainDb(c)>=3&&model.gainDb(c)<=6);
        float bed=mix::ambienceForTotal(model.gainDb(c),model.gainDb(c),model.ambienceLift,model.ceilingDb(c));
        assert(near(model.gainDb(c)+bed,6));
        p.game.phase=1;p.down=1;
        for(int i=1;i<=80;++i)model.update(c,p,i*0.05,0.6f,1);
        assert(model.anticipation>20); // First-down opening anticipation at a neutral site.
        p.game.phase=3;p.game.seconds=295;model.update(c,p,4.1,0.6f,1);assert(model.openingProfile.drive);
        p.game.phase=1;p.game.seconds=280;p.offense=1-side;model.update(c,p,4.2,0.6f,1);assert(model.openingWeight==1);
        p.game.seconds=250;model.update(c,p,4.3,0.6f,1);assert(model.openingWeight>0&&model.openingWeight<1);
        float fading=model.openingWeight;p.game.seconds=260;model.update(c,p,4.4,0.6f,1);assert(model.openingWeight==fading);
        p.game.seconds=200;model.update(c,p,4.5,0.6f,1);assert(model.openingWeight==0);
    }
    // Temporary missing fans do not weaken opening hype; permanent departures still do.
    Play p=opening();Model temporarilySparse,permanentlySparse;
    temporarilySparse.update(c,p,0,0.5f,1);permanentlySparse.update(c,p,0,0.5f,0.5f);
    assert(temporarilySparse.ambienceLift>permanentlySparse.ambienceLift);
    assert(near(permanentlySparse.ambienceLift,6*std::sqrt(0.5f)));
    // Late attaches, ordinary games and disabled profiles cannot invent a fresh opening.
    p=opening();p.game.phase=1;p.game.seconds=200;Model late;late.update(c,p,0,1);assert(late.openingWeight==0);
    p=opening();p.game.rivalry=false;Model ordinary;ordinary.update(c,p,0,1);assert(ordinary.openingWeight==0&&ordinary.ambienceLift==0);
    c.opening=0;p=opening();Model disabled;disabled.update(c,p,0,1);assert(disabled.openingWeight==0);c.opening=1;
    // No verified possession change: hold at most half of Q1, then fade for a quarter of Q1.
    Model bounded;p=opening();bounded.update(c,p,0,1);
    p.game.seconds=150;bounded.update(c,p,1,1);assert(bounded.openingWeight==1);
    p.game.seconds=112.5f;bounded.update(c,p,2,1);assert(near(bounded.openingWeight,0.5f));
    p.game.seconds=75;bounded.update(c,p,3,1);assert(bounded.openingWeight==0);
    p.game.quarter=2;p.game.seconds=300;bounded.update(c,p,4,1);assert(bounded.openingWeight==0);
    // Home offensive snaps retain a steady bed without defensive first-down chaos.
    Model home;p=opening();p.game.phase=1;p.down=1;p.offense=0;
    for(int i=0;i<120;++i)home.update(c,p,i*0.05,1);assert(home.anticipation==0&&home.ambienceLift==6);
    // All ordinary events and arbitrarily high momentum remain within six.
    for(Event event:{Event::HomeScore,Event::PickSix,Event::OpeningReturn,Event::OpeningTouchdown,Event::FourthDown,Event::Turnover,Event::UpsetWin}){
        Model m;p=opening();p.game={4,14,14,1,30,300};m.update(c,p,0,1);assert(m.event(event,0,p.game));
        m.momentum.heat=30;
        for(int i=1;i<360;++i){m.update(c,p,i*0.05,1);assert(m.gainDb(c)<=6&&m.ceilingDb(c)<=6);}
    }
    // An event observed at 61/90 seconds cannot acquire exception status later as the clock falls.
    for(float seconds:{61.0f,90.0f}){
        Model m;p=opening();p.game={4,23,20,4,seconds,300};m.update(c,p,0,1);m.event(Event::LastSecondLead,0,p.game);
        p.game.seconds=10;for(int i=1;i<100;++i){m.update(c,p,i*0.05,1);assert(m.gainDb(c)<=6);}
    }
    // A qualified last-minute winner rises to eight and releases back to the normal ceiling.
    for(Event decisive:{Event::LastSecondLead,Event::OvertimeWin,Event::GameWinningScore}){
        Model m;p=opening();p.game={4,23,20,7,60,300};if(decisive==Event::OvertimeWin){p.game.quarter=6;p.game.seconds=300;}
        Outcome o;o.accepted=true;o.event=decisive;o.sequence=1;m.update(c,p,0,1);assert(m.react(c,o,0,p.game));
        float peak=0;
        for(int i=1;i<400;++i){m.update(c,p,i*0.05,1);float gain=m.gainDb(c);peak=(std::max)(peak,gain);assert(gain<=8);if(i>=320)assert(gain<=6);}
        assert(peak>7.9f);
        m.reset();assert(m.ceilingDb(c)<=6);
    }
    Envelope protectedWin;p.game={4,23,20,7,20,300};assert(protectedWin.trigger(Event::LastSecondLead,0,p.game,1));
    assert(!protectedWin.trigger(Event::UpsetWin,9,p.game,1));assert(!protectedWin.trigger(Event::HomeScore,9,p.game,2));
    assert(protectedWin.trigger(Event::HomeScore,17,p.game,2)&&!protectedWin.special);
    c.eruptions=0;Model silent;p.game={4,23,20,7,20,300};silent.event(Event::LastSecondLead,0,p.game);
    silent.update(c,p,1,1);assert(silent.ceilingDb(c)==6);
    c=energy::Settings{};c.maxDb=0;assert(silent.ceilingDb(c)==0);
    // A time-expiring regulation field goal can jump straight to EndOfGame with a reset display clock.
    Tracker finalScore;p=opening();p.game={4,20,20,1,2,300};p.down=4;p.offense=0;
    finalScore.observe(c,p);p.game.phase=3;finalScore.observe(c,p);
    p.game.phase=7;p.game.quarter=6;p.game.seconds=300;p.game.homeScore=23;
    auto gameWinner=finalScore.observe(c,p);assert(gameWinner.event==Event::GameWinningScore&&gameWinner.completed);
    assert(finalScore.observe(c,p).event==Event::None);
    // The owning audio callback independently expires permission even if presentation stops updating.
    assert(mix::callbackCeiling(16,16,16,true,1000,0)==6);
    assert(mix::callbackCeiling(6,8,8,true,1000,1001)==8);
    assert(mix::callbackCeiling(6,8,8,true,1001,1001)==6);
    assert(mix::callbackCeiling(6,8,8,false,1000,1001)==6);
    assert(mix::callbackCeiling(6,6,8,true,1000,1001)==6);
    assert(mix::callbackCeiling(6,8,7,true,1000,1001)==7);
    assert(mix::callbackCeiling(0,8,8,true,1000,1001)==0);
    std::cout<<"PASS: pregame/first-drive opening hype, first-down anticipation, temporary vs permanent attendance, fade bounds and verified 6/8 dB event ceilings.\n";
}
