#pragma once
// Exact 2.1.11 fixture adapter, renamed for side-by-side model verification.
namespace attendance::visitors {
inline Plan fixturePlan(const turnout::CrowdFixture& fixture,int home,int away){
    if(!fixture.owns(home,away))return {};
    const auto& s=fixture.seed;Demand d;
    d.homeFans=s.home.audienceFans;d.awayFans=s.away.audienceFans;
    d.home={s.home.latitude,s.home.longitude};d.away={s.away.latitude,s.away.longitude};
    d.homeState={s.home.stateLatitude,s.home.stateLongitude};d.awayState={s.away.stateLatitude,s.away.stateLongitude};
    d.sameState=turnout::sameState(s);d.rivalry=fixture.launch.rivalry;
    const auto& host=s.home;
    d.marquee=host.apRank>=1&&host.apRank<=10&&s.away.apRank>=1&&s.away.apRank<=10;
    if(host.apRank>=1&&host.apRank<=25)d.homeStrength=.65+.35*(25-host.apRank)/24.;
    const int games=host.wins+host.losses;
    if(host.wins>=0&&host.losses>=0&&games>=4){
        const double form=(std::clamp)((double(host.wins)/games-.5)*2.,0.,1.);
        d.homeStrength=(std::max)(d.homeStrength,form*(std::min)(1.,games/8.));
    }
    d.neutral=fixture.launch.neutral;d.seed=turnout::fixtureHash(s);
    auto out=planLegacy(d);
    if(turnout::fcs(away)&&!d.neutral){out.valid=true;out.share=.01f;}
    return out;
}
}
