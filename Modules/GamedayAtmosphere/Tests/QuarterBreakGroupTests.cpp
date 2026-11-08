#include "../Source/DepartureModel.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    Settings settings;History history;State s{4,0,0,5,720,720};s.rank0=s.rank1=50;
    int previous=0;for(int margin=0;margin<=50;++margin){s.awayScore=margin;int points=departures::fourthBreakPoints(settings,s,history);assert(points>=previous&&points>=1&&points<=7);previous=points;}
    for(int margin:{0,8,14,21,28,35}){s.awayScore=margin;int losing=departures::fourthBreakPoints(settings,s,history);std::swap(s.homeScore,s.awayScore);int winning=departures::fourthBreakPoints(settings,s,history);assert(winning<=losing);std::swap(s.homeScore,s.awayScore);}
    s.awayScore=35;assert(departures::fourthBreakPoints(settings,s,history)==7);s.playoff=true;assert(departures::fourthBreakPoints(settings,s,history)<=3);s.bowl=47;assert(departures::fourthBreakPoints(settings,s,history)<=1);
    s.awayScore=7;assert(!departures::fourthBreakPoints(settings,s,history));s.playoff=false;s.bowl=-1;
    for(float length:{60.f,180.f,720.f,900.f})for(int cap:{1,45}){
        departures::Model model;State p{3,0,35,5,1,length};p.rank0=p.rank1=50;model.observe(settings,p,history);
        p.quarter=4;p.seconds=length;model.observe(settings,p,history);assert(model.fourthBreakGroup==7&&model.takeFourthBreakNotice()==7&&!model.takeFourthBreakNotice());
        model.beginWindow(100);auto result=model.settle(p,0,0,cap);assert(result.loss<=cap&&result.loss<=lossCeiling(p));
        model.observe(settings,p,history);assert(!model.takeFourthBreakNotice());auto again=model.settle(p,result.loss,0,cap);assert(again.loss==result.loss);
    }
    {departures::Model attached;State p{4,0,35,5,720,720};attached.observe(settings,p,history);assert(!attached.fourthBreakGroup);}
    {departures::Model manual;State p{3,0,35,5,1,720};manual.observe(settings,p,history,false);p.quarter=4;p.seconds=720;manual.observe(settings,p,history);assert(!manual.fourthBreakGroup);}
    {departures::Model protectedGame;history.comeback=true;State p{3,0,7,5,1,720};protectedGame.observe(settings,p,history);p.quarter=4;p.seconds=720;protectedGame.observe(settings,p,history);assert(!protectedGame.fourthBreakGroup);}
    std::puts("PASS: smooth 1-7-point Q3/Q4 group, home-winning/postseason patience, once-only crossing, quarter scaling, floors/caps, late-attach/manual/comeback guards and no repeated break penalty.");
}
