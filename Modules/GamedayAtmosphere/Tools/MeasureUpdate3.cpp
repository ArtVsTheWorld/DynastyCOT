// Read-only release facts from production policies. No game, MMC or save access.
#include "../Source/SettingsMenu.h"
#include "../Source/StartingAttendance.h"
#include "../Source/AttendanceAudio.h"
#include "../Source/ReactionCatalog.h"
#include "../Source/DirectorShake.h"
#include "../Source/LiveReactions.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <cassert>
using namespace attendance;
struct ReturnRange {int start=0,peak=0;};
ReturnRange returnRange(int type){
    live::Returns returns;energy::Play play;play.game={1,0,0,1,700,720};play.offense=1;play.down=type==22?4:0;
    uint64_t now=1000;
    auto sample=[&](int phase,float spot,bool held){play.game.phase=phase;now+=100;return returns.observe(play,1,1,now,true,type,spot,held);};
    sample(1,0,true);sample(3,0,false);sample(3,40,false);play.offense=0;
    ReturnRange range;
    for(int gain=0;gain<=45;++gain){auto outcome=sample(3,40.f-gain,true);
        if(!range.start&&returns.buildup().id)range.start=gain;if(outcome.id)range.peak=gain;}
    assert(range.start&&range.peak);return range;
}
int main(){
    std::cout<<std::setprecision(7)<<"{\n  \"runtime_version\": \"2.0.21\",\n";
    std::map<std::string,int> groups;
    for(const auto& option:settingsmenu::Options)++groups[option.group];
    std::cout<<"  \"menu\": {\"total\": "<<settingsmenu::Count<<", \"sections\": {";
    bool comma=false;for(const auto& item:groups){if(comma)std::cout<<", ";comma=true;std::cout<<'"'<<item.first<<"\": "<<item.second;}std::cout<<"}},\n";
    unsigned families=0,selectors=0;for(const auto& family:catalog::Families)families+=family.mask!=0;
    for(int id=0;id<112;++id)selectors+=catalog::usable(id);
    std::cout<<"  \"reactions\": {\"usable_selectors\": "<<selectors<<", \"usable_families\": "<<families<<", \"excluded_selectors\": [13,14,15,23]},\n";
    turnout::Team empty;empty.atmosphere=empty.tradition=empty.brand=empty.conference=12;
    std::cout<<"  \"support_weights_percent\": {";
    const char* names[]={"stadium_atmosphere","team_prestige","program_tradition","brand_exposure","conference_prestige","ap_rank"};
    for(int feature=0;feature<6;++feature){auto team=empty;
        if(feature==0)team.atmosphere=0;if(feature==1)team.prestige=10;if(feature==2)team.tradition=0;
        if(feature==3)team.brand=0;if(feature==4)team.conference=0;if(feature==5)team.apRank=1;
        if(feature)std::cout<<", ";std::cout<<'"'<<names[feature]<<"\": "<<100*turnout::support(team,138);}
    std::cout<<"},\n  \"showcase_rivalries\": [";
    turnout::Seed seed;seed.home=empty;seed.away=empty;seed.home.id=17;seed.away.id=23;seed.home.rank=seed.away.rank=130;
    seed.home.standing=seed.away.standing=0;seed.schools=138;seed.stadium=seed.homeStadium=1;seed.capacity=seed.ea=100000;seed.regular=true;seed.week=13;seed.year=2026;
    turnout::Launch launch{17,23,1,100000,100000,720,0,true,false};
    comma=false;for(int id=0;id<1024;++id){auto event=showcase::event(id);if(!event)continue;
        launch.rivalryEvent=id;auto result=turnout::calculate(seed,launch);assert(result.applied&&result.percent==100);
        if(comma)std::cout<<",";comma=true;std::cout<<"\n    {\"event\": "<<id<<", \"name\": \""<<event.name<<"\", \"peak_percent\": "<<result.percent<<", \"kickoff_low\": "<<event.low<<", \"kickoff_high\": "<<event.high<<"}";}
    std::cout<<"\n  ],\n  \"audio_examples\": [";
    comma=false;for(uint64_t seats:{6060ull,61888ull})for(float fullness:{0.f,.25f,.30f,.70f,1.f}){
        auto mix=balance::mix(fullness,seats);assert(mix.valid);auto signal=[&](unsigned child){return mix.mute?0.f:std::pow(10.f,mix.net(child)/20);};
        if(comma)std::cout<<",";comma=true;std::cout<<"\n    {\"mapped_seats\": "<<seats<<", \"fullness_percent\": "<<100*fullness<<", \"vanilla_crossover_percent\": "<<100*balance::crossover(seats)<<", \"background_signal_percent\": "<<100*signal(1)<<", \"reaction_signal_percent\": "<<100*signal(3)<<", \"individual_signal_percent\": "<<100*signal(5)<<"}";}
    std::cout<<"\n  ],\n  \"shake_curve\": [";comma=false;
    for(uint64_t occupied:{6060ull,15000ull,30000ull,50000ull,61888ull}){if(comma)std::cout<<", ";comma=true;std::cout<<"{\"occupied\": "<<occupied<<", \"multiplier\": "<<motion::occupiedScale(occupied,1)<<"}";}
    auto kickoff=returnRange(23),punt=returnRange(22);live::Settings settings;
    std::array<float,16> projection{};projection[0]=2;projection[5]=3.1715948f*2;projection[11]=1;
    std::cout<<"],\n  \"closeup_half_scale\": "<<motion::projectionScale(projection,motion::Kind::Touchdown)<<",\n";
    std::cout<<"  \"gain_thresholds_yards\": {\"run_buildup_start\": "<<live::RunShakeStartYards<<", \"run_buildup_peak\": "<<live::RunShakePeakYards<<", \"default_run_cheer\": "<<settings.minimumGainYards<<", \"default_air_catch\": "<<settings.minimumCatchYards<<", \"kickoff_start\": "<<kickoff.start<<", \"kickoff_peak\": "<<kickoff.peak<<", \"punt_start\": "<<punt.start<<", \"punt_peak\": "<<punt.peak<<"}\n}\n";
}
