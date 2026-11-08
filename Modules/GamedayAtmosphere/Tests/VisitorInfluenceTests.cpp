#include "../Source/VisitorPlans.h"
#include "../Source/SettingsMenu.h"
#include <cassert>
#include <sstream>
#include <cstdio>
using namespace attendance;
int main(){
    using namespace visitors;
    for(unsigned seed=0;seed<1000;++seed){
        Demand d;d.homeFans=100000;d.awayFans=90000;d.home={30,-97};d.away={40,-83};d.homeStrength=1;d.marquee=true;d.seed=seed;
        const auto base=plan(d);assert(influence(base,100).share==base.share);
        float prior=0;
        for(int n=0;n<=200;++n){const auto adjusted=influence(base,n);assert(adjusted.valid&&adjusted.share>=.01f&&adjusted.share<=.70f&&adjusted.share>=prior);prior=adjusted.share;}
        assert(influence(base,0).share==.01f&&influence(base,50).share<base.share&&influence(base,200).share>base.share);
        assert(influence(base,NAN).share==base.share);
        d.neutral=true;const auto neutral=plan(d);assert(influence(neutral,0).native&&influence(neutral,200).share==.5f);
    }
    Plan baseline;baseline.valid=true;baseline.share=.10f;
    PlanBook book;FixtureKey key{13,10,20,1,2,4,0,2026};Plan selected;unsigned seed=7;bool reused=false;
    assert(book.resolve(key,influence(baseline,50),7,selected,seed,reused)&&!reused);float saved=selected.share;
    assert(book.resolve(key,influence(baseline,200),8,selected,seed,reused)&&reused&&selected.share==saved&&seed==7);
    std::stringstream stream;book.write(stream);PlanBook loaded;assert(PlanBook::read(stream,loaded));
    assert(loaded.resolve(key,influence(baseline,0),9,selected,seed,reused)&&reused&&selected.share==saved);
    ++key.week;assert(loaded.resolve(key,influence(baseline,200),9,selected,seed,reused)&&!reused&&selected.share>saved);
    // Existing controls retain their numeric IDs; the new control is appended.
    const auto& option=settingsmenu::Options[settingsmenu::Count-1];
    assert(std::wcscmp(option.key,L"VisitorCrowdInfluencePercent")==0&&option.factory==100&&option.low==0&&option.high==200&&option.kind==settingsmenu::Kind::Slider);
    std::puts("PASS: 201000 visitor-influence comparisons; exact default, bounds, neutral, saved-fixture reuse/reload and new-fixture application; stable existing control IDs.");
}
