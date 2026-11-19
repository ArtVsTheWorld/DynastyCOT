#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../Source/RecruitResumeRuntime.h"
#include <cassert>
#include <iostream>

using namespace recruitingfog;
using namespace recruitingfog::resume;
using namespace recruitingfog::measurables;

static NativeClass linemanSnapshot(){
    const Json player={{"FirstName","Alex"},{"LastName","Bell"},{"Height",77},{"PresentationId",9876},
        {"Position","RG"},{"TraitDevelopment","Normal"},{"PLYR_HOME_TOWN","Salem"},{"PLYR_HOME_STATE","Oregon"},
        {"SpeedRating",81},{"AccelerationRating",88},{"StrengthRating",97},{"AgilityRating",89},
        {"ChangeOfDirectionRating",85},{"JumpingRating",80}};
    const Json raw={{"format",1},{"sourceKind","runtime"},{"status",{{"league",42},{"season",{{"CurrentSeasonYear",2026}}}}},
        {"records",{{"5:12",{{"key","5:12"},{"type","Recruit"},{"capacity",100},{"v",{{"Class","HighSchool"},{"Player","6:8"}}}}},
            {"6:8",{{"type","Player"},{"v",player}}}}}};
    return parseNativeClass(raw);
}

int main(){
    const auto data=linemanSnapshot();
    const auto& lineman=data.entries.at(5*131072+12);
    assert(lineman.prospect.position=="RG");
    const auto fields=dynasty::live::resumeBackgroundFields("Player");
    for(const auto* name:nativeFields)assert(fields.count(name)==1);

    // This archetype response contains only the Agility row.
    NotesProspect p;
    p.position="RG";p.measurementIdentity="Alex|Bell|Salem|OR|655372";p.progress=100;
    p.observations={{"Agility",89,89,true}};
    auto render=[&](const NativeEntry* entry){
        std::string text;
        appendMeasurements(text,p,42,entry);
        return text;
    };
    const auto complete=render(&lineman);
    const auto filled=notesMeasurementInputs(p,&lineman);
    assert(filled==Inputs({81.,88.,97.,85.,89.,80.}));
    assert(complete.rfind("ATHLETIC MEASURABLES\n",0)==0);
    assert(std::count(complete.begin(),complete.end(),'\n')==6);
    assert(complete.find('(')==std::string::npos);
    for(Event event:{Event::Dash,Event::Split,Event::Bench,Event::Cone,Event::Shuttle,Event::Vertical}){
        const auto reading=estimate(event,filled,seedIdentity(42,p.measurementIdentity));
        assert(complete.find(reading.label+": "+reading.value)!=std::string::npos);
    }
    const auto dash=std::stod(estimate(Event::Dash,filled,seedIdentity(42,p.measurementIdentity)).value);
    const auto split=std::stod(estimate(Event::Split,filled,seedIdentity(42,p.measurementIdentity)).value);
    const auto lifts=estimate(Event::Bench,filled,seedIdentity(42,p.measurementIdentity)).value;
    assert(dash>=4.73&&dash<=4.91&&split>=1.79&&split<=1.88);
    const auto bench=std::stoi(lifts),squat=std::stoi(lifts.substr(lifts.find('/')+1));
    assert(bench>=465&&bench<=510&&squat>=645&&squat<=720);
    assert(dash>split&&dash<4*split);
    assert(render(&lineman)==complete);
    assert(render(nullptr).find("40-Yard Dash")==std::string::npos);
    p.progress=99;
    assert(render(&lineman)==render(nullptr));
    const auto partial=render(&lineman);
    assert(partial.find("20-Yard Shuttle")!=std::string::npos&&partial.find("Bench / Squat")==std::string::npos);
    p.progress=100;
    // An explicit native range remains concealed. A malformed captured field
    // or a missing field also cannot produce a reading.
    p.observations.push_back({"Speed",70,90,false});
    assert(render(&lineman).find("40-Yard Dash")==std::string::npos);
    p.observations.pop_back();
    auto incomplete=lineman;
    incomplete.prospect.ratings["StrengthRating"]=0;
    incomplete.prospect.ratings["JumpingRating"]=80.5;
    incomplete.prospect.ratings.erase("ChangeOfDirectionRating");
    const auto missing=render(&incomplete);
    assert(missing.find("Bench / Squat")==std::string::npos&&missing.find("Vertical Jump")==std::string::npos&&missing.find("3-Cone Drill")==std::string::npos);
    p.observations.clear();
    assert(render(nullptr).empty());
    std::map<std::string,double> scouting;
    Cache::complete(lineman,scouting);
    assert(scouting.size()==2&&scouting.at("speed")==81&&scouting.at("acceleration")==88);

    // The rounded split and dash stay coherent at all integer input pairs.
    Inputs inputs;
    for(int speed=0;speed<=100;++speed)for(int acceleration=0;acceleration<=100;++acceleration){
        const auto id=seedIdentity(42,std::to_string(speed)+"|"+std::to_string(acceleration));
        inputs[Speed]=speed;inputs[Acceleration]=acceleration;
        const auto oldDash=fixed((std::clamp)(interpolate(.6*speed+.4*acceleration,6.50,4.21)+.08*noise(id,"dash"),4.15,6.60),2)+"s";
        const auto shownDash=estimate(Event::Dash,inputs,id),shownSplit=estimate(Event::Split,inputs,id);
        assert(shownDash.value==oldDash);
        const auto full=std::stod(shownDash.value),first=std::stod(shownSplit.value);
        assert(first>=1.65&&first<=2.52&&full>first&&full<4*first);
    }
    constexpr std::array<Event,4> timed={Event::Dash,Event::Split,Event::Cone,Event::Shuttle};
    for(int r=1;r<=100;++r){
        inputs.fill(static_cast<double>(r));
        for(const auto event:timed){
            const auto value=estimate(event,inputs,"42|timing-"+std::to_string(r)).value;
            const auto time=std::stod(value);
            const auto& spec=events[static_cast<size_t>(event)];
            assert(value.size()==5&&value[1]=='.'&&value.back()=='s'&&time>=spec.minimum&&time<=spec.maximum);
        }
        const auto value=estimate(Event::Bench,inputs,"42|lifts-"+std::to_string(r)).value;
        const auto b=std::stoi(value),sq=std::stoi(value.substr(value.find('/')+1));
        assert(b%5==0&&sq%5==0&&b>=80&&b<=520&&sq>=110&&sq<=735&&sq>b);
        const auto vertical=std::stod(estimate(Event::Vertical,inputs,"42|jump-"+std::to_string(r)).value);
        assert(vertical>=10.8&&vertical<=46.2);
    }
    std::cout<<"FULLY SCOUTED AGILE RG\n"<<complete<<'\n';
    std::cout<<"PASS verified field selection and identity fixture, archetype omissions, partial gates, missing inputs, old Scouting supplement, formula bounds and dash/split coherence.\n";
}
