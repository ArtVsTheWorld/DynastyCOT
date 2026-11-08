// Offline deterministic harness. Includes production policies; never attaches to
// the game or reads/writes installed files. Scores and possession are scenarios,
// not predicted games. Visual density approximates native masks as peak/capacity.
#include "../Source/DepartureModel.h"
#include "../Source/ArrivalRules.h"
#include "../Source/HalftimeExitConversion.h"
#include "../Source/LineupAtmosphere.h"
#include "../Source/VenueMotion.h"
#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
using namespace attendance;
struct Fixture {std::string name;float standing=.5f;int rank=60,opponent=65,rain=0,path=0;bool night=false,rivalry=false,severe=false,neutral=false;int postseason=0;};
struct Point {float t=0,fullness=0,ideal=0;int loss=0,temporary=0,home=0,away=0;bool ordinary=false,critical=false,opening=false,late=false,upset=false;float nativePeak=0;};
struct Run {turnout::Result peak;std::vector<Point> points;int finalLoss=0;float finalFullness=0;};
void scoreboard(State& s,int tick,int path){
    // Monotonic score paths, at explicit reference-clock moments.
    s.homeScore=s.awayScore=0;
    if(path==0){for(int t:{300,900,1500,2100})if(tick>=t)s.homeScore+=7;for(int t:{600,1200,1800})if(tick>=t)s.awayScore+=7;}
    if(path==1){for(int t:{300,600,1000,1400,1800,2200})if(tick>=t)s.awayScore+=7;if(tick>=1200)s.homeScore=7;}
    if(path==2){for(int t:{300,650,1000,1350,1700,2100})if(tick>=t)s.homeScore+=7;if(tick>=900)s.awayScore=7;}
    if(path==3){for(int t:{250,600,950,1500})if(tick>=t)s.awayScore+=7;for(int t:{1300,1700,2000,2300})if(tick>=t)s.homeScore+=7;}
    if(path==4){for(int t:{300,900,1500,2300})if(tick>=t)s.homeScore+=7;for(int t:{600,1300,1900})if(tick>=t)s.awayScore+=7;}
    if(path==5){s.homeScore=tick>=600?14:tick>=300?7:0;s.awayScore=tick>=900?14:tick>=400?7:0;for(int t:{2200,2400,2600})if(tick>=t)s.awayScore+=7;}
    if(path==10){for(int t:{300,730,1000,1350,1700,2100})if(tick>=t)s.homeScore+=7;for(int t:{900,1500,2300})if(tick>=t)s.awayScore+=3;}
    if(path>=6&&path<=9){bool home=path>=8,early=path%2==0;int winner=0;
        for(int t:{300,600,750,900,1050,1200})if(tick>=(early?t:2400+(t-300)/3))winner+=7;
        s.homeScore=home?winner:(tick>=600?7:0);s.awayScore=home?(tick>=600?7:0):winner;}
}
Run simulate(const Fixture& f,uint64_t seats,int length=720,int window=30,bool detailed=false,unsigned variationSeed=12345){
    turnout::Seed seed;seed.home={f.severe?1218:1153,f.rank,70,6,3,3,3,6,6,6,6,f.standing};seed.away={1193,f.opponent,80,5,3,3,3,6,6,6,6,.5f};
    seed.stadium=seed.homeStadium=553;seed.capacity=60000;seed.schools=138;seed.week=8;seed.year=2026;seed.ea=60000;seed.regular=!f.postseason;
    turnout::Launch launch{seed.home.id,seed.away.id,553,60000,60000,f.night?1200:720,f.rain,f.rivalry,f.neutral};
    Run run;run.peak=turnout::calculate(seed,launch);auto& peak=run.peak;
    State s{1,0,0,5,float(length),float(length)};s.team0=seed.home.id;s.team1=seed.away.id;s.rank0=f.rank;s.rank1=f.opponent;
    s.rivalry=f.rivalry;s.neutral=f.neutral;s.precipitation=f.rain;s.variationSeed=variationSeed;s.night=f.night;
    if(f.postseason){s.bowl=f.postseason==2?47:40;s.playoff=f.postseason!=3;peak.percent=100;}
    else {assert(peak.applied);s.startingAttendance=true;s.plannedKickoff=peak.kickoff;s.plannedFill=peak.fillPercent;s.plannedPeak=peak.attendance;s.plannedCapacity=60000;s.plannedStadium=553;s.plannedWeek=8;s.plannedYear=2026;s.exceptionalOpening=peak.exceptional;}
    Settings c;arrivals::Settings a;History history;departures::Model departures;arrivals::Model arrivals;arrivals::Halftime halftime;halftime::ExitConversion conversion;
    core::Settings atmosphere;int permanent=0,visibleMissing=0;
    for(int tick=0;tick<=2880;++tick){
        float time=tick*length/720.f;s.quarter=(std::min)(4,tick/720+1);s.seconds=s.quarter*length-time;
        s.offense=(tick/120)%2;s.homeTimeouts=tick>=2600?0:3;s.toGoal=60;scoreboard(s,tick,f.path);
        assert(history.observe(c,s));assert(departures.observe(c,s,history));
        if(tick%window==0||tick==2880){
            int before=permanent;
            departures.beginWindow(s,c.transitionCap);departures.anchorVisible(permanent,visibleMissing);
            auto arrival=arrivals.observe(a,s);if(arrival.starting)departures.openingVisible();
            auto temporary=[&](){bool packed=packed::qualifies(seats,peak.percent*.01f*(100-permanent)*.01f);
                int bathroom=halftime.observe(s,permanent,100,packed);if(halftime.releaseAtBreak)departures.halftimeVisible();
                visibleMissing=departures.paceTemporary(permanent,(std::max)(arrivals.missing,bathroom));
                if(s.quarter==3&&halftime.done&&!arrivals.missing)visibleMissing=0;
                if(s.quarter==3)halftime.lastAbsent=(std::min)(halftime.lastAbsent,(std::max)(0,visibleMissing-permanent));};
            temporary();permanent=departures.settle(s,permanent,0,c.quarterCap).loss;if(permanent!=before)temporary();
            int total=(std::max)(permanent,visibleMissing),wanted=conversion.request(s,permanent,total,25);
            if(wanted>0){int after=departures.convertSeatBreakers(s,permanent,wanted,c.quarterCap).loss;if(after>permanent){halftime.convert(s,permanent,after,total,100);permanent=after;}}
            assert(permanent>=before&&permanent<=(std::max)(before,lossCeiling(s)));
            float density=peak.percent*.01f*(100-(std::max)(permanent,visibleMissing))*.01f;
            assert(std::isfinite(density)&&density>=0&&density<=peak.percent*.01f+.00001f);
            if(detailed){Point p;p.t=static_cast<float>(tick);p.fullness=density*100;p.loss=permanent;p.temporary=(std::max)(0,visibleMissing-permanent);p.home=s.homeScore;p.away=s.awayScore;
                auto ideal=arrivals::missingAt(100-arrivals::openingPercent(a,s),float(tick),720*arrivals::finishFraction(a,s),s);
                p.ideal=peak.percent*(100-ideal)*.01f;
                energy::Play probe;probe.game=s;probe.game.phase=1;probe.offense=s.offense;probe.down=1;probe.toFirst=10;probe.toGoal=60;
                bool firstDrive=tick<150;
                p.opening=firstDrive&&density>=lineup::openingMinimum(s,seats)&&lineup::competitive(s)>0;
                p.ordinary=lineup::cameraAdmission(atmosphere,probe,density,true,seats,3,false,firstDrive).allowed;
                probe.down=3;p.critical=lineup::cameraAdmission(atmosphere,probe,density,true,seats,3,false,firstDrive).allowed;
                probe.down=1;probe.toGoal=30;probe.offense=0;p.late=lineup::cameraAdmission(atmosphere,probe,density,true,seats,3,false,false).allowed&&lineup::lateDrive(probe)>0;
                p.upset=upsetsound::evaluate(s,seats,density,true,true).qualified;
                p.nativePeak=1000; // Native excitement is no longer reduced; bus gain is separate.
                assert(p.nativePeak>=0&&p.nativePeak<=1000);assert(density>=core::ReactionMinimumFullness||(!p.ordinary&&!p.critical&&!p.opening&&!p.late));
                run.points.push_back(p);
            }
        }
    }
    run.finalLoss=permanent;run.finalFullness=peak.percent*(100-(std::max)(permanent,visibleMissing))*.01f;return run;
}
int main(int argc,char** argv){
    if(argc<2)return 2;std::filesystem::path dir=argv[1];std::filesystem::create_directories(dir);
    std::vector<Fixture> fixtures={
        {"Bottom support close",0,135,137},{"Low support close",.05f,120,110},{"Lower middle close",.25f,95,90},
        {"Middle support close",.5f,60,65},{"Upper support close",.75f,35,45},{"Elite ordinary close",.95f,8,55},
        {"Elite night rivalry",1,3,8,0,0,true,true},{"Severe late elite night",.95f,8,55,0,0,true,false,true},
        {"Middle rain home rout",.5f,60,65,1,2},{"Elite rain home rout",.95f,8,100,1,2},
        {"Middle home loss",.5f,60,55,0,1},{"Middle rain home loss",.5f,60,55,1,1},
        {"Late growing deficit",.75f,35,45,0,5},{"Comeback",.75f,35,45,0,3},
        {"Upset bid",.5f,60,8,0,4},{"Neutral rivalry",.85f,13,36,0,0,false,true,false,true},
        {"CFP blowout",.95f,8,4,0,1,false,false,false,true,1},
        {"Championship blowout",.95f,8,4,0,1,false,false,false,true,2},
        {"Bowl close native peak",.75f,35,45,0,0,false,false,false,true,3},
        {"Early 35-point home loss",.5f,60,55,0,6},{"Late 35-point home loss",.5f,60,55,0,7},
        {"Early rainy 35-point home win",.5f,60,65,1,8},{"Late rainy 35-point home win",.5f,60,65,1,9},
        {"Night home win42-9",.95f,35,60,0,10,true},
        {"Day home win42-9",.95f,35,60,0,10,false},
        {"Neutral night win42-9",.95f,35,60,0,10,true,false,false,true}};
    std::ofstream points(dir/"game-curves.csv"),summary(dir/"scenario-summary.csv"),checks(dir/"checks.txt"),sensitivity(dir/"sensitivity.csv"),inputs(dir/"fixture-inputs.csv");
    inputs<<"scenario,support_percentile,home_rank,away_rank,precipitation,score_path,night,rivalry,severe_late,neutral,postseason_kind,nominal_capacity\n";
    sensitivity<<"scenario,seats,quarter_length,hidden_window_reference_seconds,final_departures_pct_of_peak,final_capacity_pct\n";
    points<<"scenario,seats,elapsed_reference_seconds,fullness_pct,ideal_arrivals_only_pct,permanent_departures_pct_of_peak,temporary_absences_pct_of_peak,home,away,ordinary_down_camera,critical_down_camera,opening_route,late_drive_route,upset_sound_route,native1000_output,occupied_seats,crowd_motion_factor\n";
    summary<<"scenario,seats,peak_capacity_pct,kickoff_share_of_peak,final_capacity_pct,final_departures_pct_of_peak,size_factor,first_ordinary_time,first_critical_time,quarter_length,window_interval\n";
    unsigned games=0;
    for(const auto& f:fixtures)for(uint64_t seats:{6060ull,30000ull,61888ull}){
        if(seats==6060)inputs<<f.name<<','<<100*f.standing<<','<<f.rank<<','<<f.opponent<<','<<f.rain<<','<<f.path<<','<<f.night<<','<<f.rivalry<<','<<f.severe<<','<<f.neutral<<','<<f.postseason<<",60000\n";
        auto r=simulate(f,seats,720,30,true);float ordinary=-1,critical=-1;
        for(const auto& p:r.points){if(p.ordinary&&ordinary<0)ordinary=p.t;if(p.critical&&critical<0)critical=p.t;
            points<<f.name<<','<<seats<<','<<p.t<<','<<p.fullness<<','<<p.ideal<<','<<p.loss<<','<<p.temporary<<','<<p.home<<','<<p.away<<','<<p.ordinary<<','<<p.critical<<','<<p.opening<<','<<p.late<<','<<p.upset<<','<<p.nativePeak<<','<<seats*p.fullness*.01f<<','<<motion::occupiedScale(seats,p.fullness*.01f)<<'\n';}
        int opening=f.postseason?static_cast<int>(std::lround(r.points.front().fullness)):r.peak.kickoff;
        summary<<f.name<<','<<seats<<','<<r.peak.percent<<','<<opening<<','<<r.finalFullness<<','<<r.finalLoss<<','<<motion::venueScale(seats)<<','<<ordinary<<','<<critical<<",720,30\n";
        for(int length:{60,300,720,1200})for(int cadence:{10,30,120}){
            auto other=simulate(f,seats,length,cadence);assert(other.finalLoss>=0&&other.finalLoss<=85);++games;
            sensitivity<<f.name<<','<<seats<<','<<length<<','<<cadence<<','<<other.finalLoss<<','<<other.finalFullness<<'\n';
            // Different reference cadence may delay visible windows. Never
            // assert identical halftime rendering; retain the actual values.
            if(cadence==30&&length==720)assert(std::abs(other.finalFullness-r.finalFullness)<.001f);
        }
    }
    checks<<"Production-policy full-game runs: "<<games<<"\nScenarios: "<<fixtures.size()<<"\nVenues: 6060, 30000, 61888 authored positions\nQuarter lengths: 60, 300, 720, 1200 seconds\nHidden windows: every 10, 30, 120 reference seconds\nAssertions: finite bounded fullness, monotonic permanent exits, crowd floors, native peak bounds, no motion below absolute 50%, deterministic repeat.\n";
    std::ofstream variety(dir/"halftime-variation.csv");variety<<"seed,departure_start_Q2_seconds_left,main_return_Q3_seconds_elapsed,last_return_Q3_seconds_elapsed,halftime_seated_pct,final_seated_pct\n";
    for(unsigned seed=1;seed<=128;++seed){State timing;timing.variationSeed=seed;auto t=context::halftimeTiming(timing);auto r=simulate(fixtures[3],61888,720,30,true,seed);
        auto half=std::find_if(r.points.begin(),r.points.end(),[](const Point& p){return p.t==1440;});assert(half!=r.points.end());
        variety<<seed<<','<<t.departure*720<<','<<t.mainReturn*720<<','<<t.lastReturn*720<<','<<half->fullness<<','<<r.finalFullness<<'\n';}
    if(argc>2){std::ifstream cohort(argv[2]);std::string line;std::vector<turnout::Team> teams;
        while(std::getline(cohort,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream in(line);turnout::Team t;if(in>>t.id>>t.rank>>t.toughest>>t.prestige>>t.wins>>t.losses>>t.homeGames>>t.atmosphere>>t.tradition>>t.brand>>t.conference>>t.apRank)teams.push_back(t);}
        assert(turnout::assignStandings(teams));std::ofstream dist(dir/"cohort-peak-sweep.csv");dist<<"support_percentile,home_rank,away_rank,rivalry,night,rain,peak_pct,kickoff_share,capacity_at_kickoff\n";unsigned count=0;
        for(const auto& home:teams)for(const auto& away:teams)if(home.id!=away.id)for(int variant=0;variant<4;++variant){
            turnout::Seed s;s.home=home;s.away=away;s.regular=true;s.stadium=s.homeStadium=553;s.capacity=60000;s.schools=static_cast<int>(teams.size());s.week=8;s.year=2026;s.ea=60000;
            turnout::Launch l{home.id,away.id,553,60000,60000,variant==1?1200:720,variant==2?1:0,variant==3,false};auto r=turnout::calculate(s,l);
            assert(r.applied&&r.percent>=(variant==2?20:25)&&r.percent<=100&&r.kickoff>=(variant==2?35:40)&&r.kickoff<=80);
            dist<<100*r.standing<<','<<home.rank<<','<<away.rank<<','<<l.rivalry<<','<<r.night<<','<<l.precipitation<<','<<r.percent<<','<<r.kickoff<<','<<r.percent*r.kickoff*.01f<<'\n';++count;
        }checks<<"Peak/formula cohort cases: "<<count<<" using "<<teams.size()<<" captured-school inputs (identities omitted in output).\n";
    }
    if(!points||!summary||!checks)return 3;std::printf("PASS: %u full-game production-policy simulations; reports in %s\n",games,argv[1]);
}
