#pragma once
#include <cstdint>
#include <string_view>

namespace recruitingfog::evidence {
enum class NoteRole {Unknown,Qb,Halfback,Fullback,Receiver,TightEnd,Tackle,Guard,Center,Edge,Interior,InsideLb,OutsideLb,Sam,Will,Corner,FreeSafety,StrongSafety,Kicker,Punter,Snapper,Athlete};
inline NoteRole noteRole(std::string_view p){
 using R=NoteRole;
 if(p=="QB")return R::Qb;
 if(p=="HB"||p=="RB")return R::Halfback;
 if(p=="FB")return R::Fullback;
 if(p=="WR")return R::Receiver;
 if(p=="TE")return R::TightEnd;
 if(p=="LT"||p=="RT")return R::Tackle;
 if(p=="LG"||p=="RG")return R::Guard;
 if(p=="C")return R::Center;
 if(p=="LE"||p=="RE"||p=="LEDG"||p=="REDG")return R::Edge;
 if(p=="DT")return R::Interior;
 if(p=="MIKE"||p=="MLB")return R::InsideLb;
 if(p=="LOLB"||p=="ROLB")return R::OutsideLb;
 if(p=="SAM")return R::Sam;
 if(p=="WILL")return R::Will;
 if(p=="CB")return R::Corner;
 if(p=="FS")return R::FreeSafety;
 if(p=="SS")return R::StrongSafety;
 if(p=="K")return R::Kicker;
 if(p=="P")return R::Punter;
 if(p=="LS")return R::Snapper;
 if(p=="ATH"||p=="Athlete"||p=="ATHLETE")return R::Athlete;
 return R::Unknown;
}
constexpr unsigned roles(NoteRole r){return 1u<<static_cast<unsigned>(r);}
constexpr unsigned operator|(NoteRole a,NoteRole b){return roles(a)|roles(b);}
constexpr unsigned operator|(unsigned a,NoteRole b){return a|roles(b);}
using NR=NoteRole;
inline constexpr unsigned noteBacks=NR::Halfback|NR::Fullback;
inline constexpr unsigned noteLine=NR::Tackle|NR::Guard|NR::Center;
inline constexpr unsigned noteLbs=NR::InsideLb|NR::OutsideLb|NR::Sam|NR::Will;
inline constexpr unsigned noteDbs=NR::Corner|NR::FreeSafety|NR::StrongSafety;
inline constexpr unsigned noteKickers=NR::Kicker|NR::Punter;

inline bool oneOfNotes(std::string_view names,std::string_view key){
 if(key.empty())return false;
 for(size_t start=0;start<names.size();){
  const auto end=names.find('|',start);
  if(names.substr(start,end==names.npos?end:end-start)==key)return true;
  if(end==names.npos)break;
  start=end+1;
 }
 return false;
}

inline std::string_view roleTraits(NoteRole role){
 using R=NoteRole;
 switch(role){
 case R::Qb:return "throw power|short accuracy|medium accuracy|deep accuracy|throw on run|under pressure|break sack|carrying";
 case R::Halfback:return "ball carrier vision|carrying|break tackle|trucking|elusiveness|catching|pass block";
 case R::Fullback:return "ball carrier vision|run block|impact blocking|pass block|carrying|catching|break tackle|trucking";
 case R::Receiver:return "release|catching|catch in traffic|spectacular catch|short route running|medium route running|deep route running|run block|carrying";
 case R::TightEnd:return "release|catching|catch in traffic|spectacular catch|short route running|medium route running|deep route running|run block|pass block|carrying";
 case R::Tackle:case R::Guard:case R::Center:return "run block|run block power|run block finesse|pass block|pass block power|pass block finesse|impact blocking";
 case R::Edge:return "tackle|block shedding|power moves|finesse moves|pursuit|play recognition|acceleration";
 case R::Interior:return "tackle|block shedding|power moves|finesse moves|play recognition|strength";
 case R::InsideLb:case R::OutsideLb:case R::Sam:case R::Will:return "tackle|hit power|block shedding|power moves|finesse moves|pursuit|play recognition|man coverage|zone coverage";
 case R::Corner:case R::FreeSafety:case R::StrongSafety:return "man coverage|zone coverage|press|tackle|hit power|pursuit|play recognition|catching";
 case R::Kicker:case R::Punter:return "kick power|kick accuracy";
 case R::Snapper:return "long snap";
 case R::Athlete:return "short accuracy|deep accuracy|throw power|catching|catch in traffic|short route running|release|ball carrier vision|break tackle|man coverage|zone coverage|tackle";
 default:return {};
 }
}

inline bool roleEligible(NoteRole role,std::string_view key){
 if(oneOfNotes(roleTraits(role),key))return true;
 if(oneOfNotes("awareness|awareness qb|awareness line|awareness receiver|awareness back|awareness defense|awareness specialist",key))return true;
 if((noteKickers|roles(NR::Snapper))&roles(role))return false;
 return oneOfNotes("speed|acceleration|strength|agility|change of direction",key);
}

inline int roleWeight(NoteRole role,std::string_view key,int emphasis){
 using R=NoteRole;
 if(role==R::Qb&&oneOfNotes("under pressure|awareness qb|throw on run",key))return 24;
 if(role==R::Halfback&&key=="ball carrier vision")return 28;
 if((role==R::Receiver||role==R::TightEnd)&&key=="release"&&!emphasis)return -22;
 if(role==R::Fullback&&oneOfNotes("run block|impact blocking|pass block",key))return 24;
 if(role==R::Center&&key=="awareness line")return 50;
 if((role==R::Guard||role==R::Center)&&key=="strength")return -26;
 if(role==R::Edge&&key=="block shedding")return 18;
 if(role==R::Interior&&key=="block shedding")return 25;
 if((role==R::Interior||role==R::Guard||role==R::Center)&&oneOfNotes("speed|acceleration|agility|change of direction",key))return -40;
 if(noteLbs&roles(role)){
  if(oneOfNotes("power moves|finesse moves",key))return emphasis?(role==R::Sam?2:-22):-95;
  if(role==R::InsideLb&&oneOfNotes("play recognition|tackle",key))return 24;
  if(role==R::OutsideLb&&oneOfNotes("block shedding|pursuit",key))return 18;
  if(role==R::Sam&&oneOfNotes("block shedding|man coverage",key))return 18;
  if(role==R::Will&&oneOfNotes("pursuit|zone coverage",key))return 23;
 }
 if(role==R::Corner&&oneOfNotes("man coverage|press",key))return 18;
 if(role==R::FreeSafety&&oneOfNotes("zone coverage|play recognition",key))return 22;
 if(role==R::StrongSafety&&oneOfNotes("tackle|zone coverage",key))return 18;
 if((role==R::FreeSafety||role==R::StrongSafety)&&key=="press"&&!emphasis)return -30;
 return 0;
}

struct RoleNoteLine {unsigned positions;const char* key;const char* lines[3];};
inline constexpr RoleNoteLine roleNotes[]={
 {roles(NR::Qb),"under pressure",{"I'd let him work through the full read even when the pocket gets crowded.","Give him a clean first answer. I'd still check how he handles the late rush.","Get the ball out early. Pressure makes him rush throws he usually completes."}},
 {roles(NR::Qb),"throw on run",{"I'd keep the boot game in the plan. He can throw it without stopping to reset.","The rollout is worth a look. Give him a clean throwing lane first.","Keep him in the pocket when we need the throw on time. The ball wanders when his feet are moving."}},
 {roles(NR::Qb),"awareness qb",{"I trust him to find a second answer when the defense takes away the first one.","I'd start with a clear first read and give him more as he gets comfortable.","Don't give him the whole menu yet. A late rotation can take him off his read."}},
 {noteBacks,"ball carrier vision",{"He sees the crease early enough to hit it before it closes. I'd trust him on inside zone.","He can follow a clean lead block. I want another look when the first lane closes.","I'd give him a clear landmark. He can miss the crease while he's looking for a better one."}},
 {roles(NR::Halfback),"catching",{"I'd keep him in on passing downs. Give him more than the checkdown and see what he does with it.","Can handle a simple outlet. I'd keep the routes short to start.","I wouldn't make him our passing-down back yet. Have to see him finish the easy outlet."}},
 {roles(NR::Halfback),"pass block",{"I can leave him in to protect on third down. He gives the quarterback a chance to finish the read.","Can help in protection with a clear assignment. I'd watch him on late pressure.","I'd keep help in the protection plan. Don't count on him to stop a free rusher alone."}},
 {roles(NR::Fullback),"run block",{"I'd put him in front of the back when we need a hard yard.","Can lead through a clean lane. I'd give him a clear first target.","I wouldn't make him the lead blocker yet. We need him to finish the block in the hole."}},
 {roles(NR::Fullback),"impact blocking",{"Give him somebody to find on the move. The first hit creates room behind him.","Can get square when the path is clear. Want to see more when the target moves.","I'd keep the first block close to home. He loses too much when he has to find contact on the move."}},
 {NR::Receiver|NR::TightEnd,"release",{"I'd leave him on the line against press. He can get into the route on time.","Can get off with a clean first step. Motion would help against a physical corner.","Get him off the line or put him in motion. Press can ruin the route before it starts."}},
 {roles(NR::Center),"awareness line",{"I'd let him help set the protection. He's quick to see where the extra rusher is coming from.","Can handle the straightforward calls. I'd have a guard help with late changes.","I wouldn't hand him all the protection calls yet. Late movement can leave his group guessing."}},
 {roles(NR::Guard),"pass block power",{"He can keep a power rusher out of the quarterback's lap. That's the kind of guard we need.","Can sit down on a straight rush when he's square. Needs to do it more consistently.","Keep help inside. A strong tackle can walk him back before the quarterback can step up."}},
 {roles(NR::Edge),"block shedding",{"He can get off the tackle's hands without giving away the edge.","He can keep the edge when he gets his hands in first. I'd watch the second effort.","Don't leave him alone on the edge yet. Tackles can tie him up and move the run outside."}},
 {roles(NR::Interior),"block shedding",{"I'd put him over a guard and let him play the run. He can get clear when the back commits.","Can separate from one blocker when he gets set. Want to see him handle less favorable angles.","I'd be careful asking him to hold the middle. A guard who gets to his chest can keep him there."}},
 {roles(NR::Interior),"power moves",{"I'd send him through the inside shoulder. He can drive a guard back into the throw.","There's some push through the middle. I'd keep him in the rotation and see if it holds up.","I wouldn't count on him to collapse the pocket. Guards can sit down on his rush."}},
 {roles(NR::InsideLb),"play recognition",{"I'd trust him in the middle of the defense. He finds the ball without chasing the first fake.","Can make a clean read on a straight run. I'd watch the misdirection reps.","Keep the read simple. If the backfield shows him two things he can open up the wrong gap."}},
 {roles(NR::Sam),"block shedding",{"I like him on the strong side. He can get clear of the block and keep the run from bouncing.","Can hold up when he gets the first touch. I'd give him a clear edge responsibility.","A tight end can keep him stuck. I'd keep the front from leaving him alone against the block."}},
 {roles(NR::Will),"pursuit",{"I'd keep him free on the weak side. He can get across the formation and close the cutback.","Takes a usable angle when he sees the ball early. I'd watch the backside pursuit.","The cutback can get away from him. Keep his first assignment simple and clean up his angles."}},
 {noteLbs,"zone coverage",{"I'd leave him in for passing downs. He can protect the space behind the front without chasing the first route.","Can handle a short drop with a clear landmark. I'd keep the exchanges simple.","I wouldn't ask him to sort several routes at once yet. Start with one clear drop."}},
 {roles(NR::Corner),"press",{"I can put him up on the receiver and trust him to make the route start late.","Can get a hand on the receiver when the release is clean. I'd mix in some off coverage.","Back him off for now. A receiver who wins the first move can leave him reaching."}},
 {roles(NR::FreeSafety),"zone coverage",{"I'd trust him over the top. He sees the routes developing without giving away the deep ball.","Can play a clear deep zone. I want another look when two receivers threaten his side.","I'd keep a second safety high with him. The deep routes can pull him out of position."}},
 {roles(NR::StrongSafety),"tackle",{"I want him down near the line. He gets square and finishes without needing the big hit.","Can fill downhill if the angle is clean. I'd check his finish against a back in space.","I wouldn't count on him as our extra run defender yet. He has to finish more cleanly."}},
 {noteDbs,"catching",{"He can make a play on the ball when he's in position. I'd give him chances to read the throw.","He can finish a clean chance. I want to see what he does with a tougher one.","I'd coach him to get the breakup first. Being in position doesn't always turn into a catch."}},
 {roles(NR::Kicker),"kick power",{"I'd give him a chance from outside our usual range. He's got the leg for it.","The leg is fine in a comfortable range. I'd be picky about the longer tries.","I wouldn't stretch his range. Take the points only when we've given him a reasonable spot."}},
 {roles(NR::Kicker),"kick accuracy",{"I'd feel good sending him out for the kick we have to make. His line holds up.","The routine kick looks fine. I'd want a closer look from the hashes.","I'd want a longer look before handing him the kicking job. Even the routine tries can drift."}},
 {roles(NR::Punter),"kick power",{"I can use his leg to flip the field when we're backed up.","Can get us out of trouble with clean contact. I wouldn't ask him to win on distance alone.","He isn't going to bail us out from our own end on leg strength. I'd ask him to win with placement."}},
 {roles(NR::Punter),"kick accuracy",{"I'd trust him to put the ball near the boundary and make the return harder.","Can hit a clear target. We need a closer look at the shorter placement punts.","I wouldn't make the sideline the whole plan. When he misses the line it can give the returner room."}},
 {roles(NR::Snapper),"long snap",{"I'd put him in the kicking operation right away. The snap gives the unit a clean start.","The snap looks usable. I want to see it hold up when we have to get the kick off quickly.","I'd keep the job open. An off-line snap can put the whole operation behind."}},
 {roles(NR::Snapper),"awareness specialist",{"Can get the unit set and stay with the operation when things get hurried.","The regular setup suits him. Check how he adjusts when the timing changes.","Give him clear timing and calls. He can be late getting comfortable when the unit has to rush."}},
 {roles(NR::Athlete),"release",{"I'd give him a look at receiver. He can get into the route without a free release.","Worth a look off the line on offense. I'd check him against press first.","If we try him at receiver, start him off the line. Press could take away his first step."}},
};

inline constexpr RoleNoteLine specialistVariants[]={
 {roles(NR::Kicker),"kick power",{"Leg gives us points on drives that stall outside our normal range.","Give him a sensible spot and he's got enough leg. I wouldn't chase extra range yet.","I'd keep the long field goals out of his early workload. Make the offense earn a better spot."}},
 {roles(NR::Kicker),"kick power",{"We can discuss the long try with him instead of treating it as an automatic punt.","Enough leg to work with. I'd leave some cushion at the edge of his range.","Take distance off the ask. I don't want his first job to be winning with leg strength."}},
 {roles(NR::Kicker),"kick accuracy",{"I'd take the points when the drive stalls. The placement is what sells me.","I'd let him work in a comfortable range. Want a bigger sample when the angle gets awkward.","Keep competition open. Making the ordinary kick consistently is the first thing he has to show."}},
 {roles(NR::Kicker),"kick accuracy",{"We can trust him with the routine points. That's a bigger deal than one long make.","He's in the mix for the job. I'd check repeatability before we give him every kick.","I wouldn't build the kicking plan around him yet. The line changes more than I'd like."}},
 {roles(NR::Punter),"kick power",{"I'd use him when we need to get the ball out of our own end. There's real distance there.","He's got enough distance for an ordinary punt. Let the coverage team do its job.","Don't expect him to erase bad field position with one swing. Give the coverage unit a clear plan."}},
 {roles(NR::Punter),"kick power",{"He's got the leg to make a short field long again.","The distance is usable. I'd check whether he can keep it when he's working faster.","I wouldn't ask for full-field distance from him. Let him aim for the return we can cover."}},
 {roles(NR::Punter),"kick accuracy",{"He gives us a chance to choose where the return starts. That's what I'd recruit him for.","He can hit a generous landmark. Tighten the target once we've seen more of him.","I'd keep his first targets wide. The miss is too costly if we ask him to paint the sideline."}},
 {roles(NR::Punter),"kick accuracy",{"Give him a chance to pin somebody deep. He can put the kick where we're asking for it.","The placement is workable. I want more confidence before using him on a short field.","I'd send the coverage team with room to adjust. Can't assume the punt lands on its mark."}},
 {roles(NR::Snapper),"long snap",{"I like him for the job. He gives the holder a clean ball to work with.","Can handle the ordinary snap. I'd check it with the rest of the unit before we settle the job.","I wouldn't set the unit around him yet. Too much depends on the ball arriving clean."}},
 {roles(NR::Snapper),"long snap",{"The ball is where the operation needs it. I'd get him reps with both units.","Worth another look with the punter. The snap is good enough to stay in the mix.","Make the snap the first thing we work on. The rest of the unit can't fix an off-target ball every time."}},
 {roles(NR::Snapper),"awareness specialist",{"I'd trust him to stay on the right timing when we have to get the unit out fast.","Knows his job in a normal setup. Need a few hurry-up reps before I'm sold.","Keep the operation simple at first. The timing can get away from him when it changes."}},
 {roles(NR::Snapper),"awareness specialist",{"Can keep his part of the operation intact when the call changes on him.","Handles the calls he knows. Let's see a less familiar setup.","Give him one clear call while he gets the timing down."}},
 {roles(NR::Kicker),"awareness specialist",{"I'd trust him to stay with the kick when the timing gets squeezed.","The normal routine suits him. Get a look when the clock puts the unit in a hurry.","I wouldn't rush his routine early. Give the whole unit a clear call."}},
 {roles(NR::Kicker),"awareness specialist",{"Looks ready for a change in the call without losing track of his setup.","He's in step with a settled operation. I'd see what happens when the spot changes late.","Keep the operation settled first. He can lose his setup when things change."}},
 {roles(NR::Punter),"awareness specialist",{"Can keep track of the coverage call and still get himself set to punt.","Can handle a clear punt call. I'd look at how he adjusts when the unit has to hurry.","Keep his calls simple. He can get caught between the coverage change and the snap."}},
 {roles(NR::Punter),"awareness specialist",{"I'd trust him to keep the unit on the right job when the look across from them changes.","He follows a settled plan. Want to see him recognize the unusual looks.","I'd have the unit make the adjustment clear. Don't leave him to sort out a late change alone."}},
};

inline constexpr RoleNoteLine fieldVariants[]={
 {roles(NR::Qb),"under pressure",{"I'd keep the intermediate throws available when the rush is coming. He can get the ball out without panicking.","He can manage the first wave. I want to see him finish the read when the pocket closes late.","Keep a quick outlet in the call. A closing pocket can take him off the throw too early."}},
 {roles(NR::Qb),"under pressure",{"I trust him to stay in the throw with a rusher in his face.","Can make the throw when he knows where the rush is coming from. A late look is a different ask.","Get him protection he can trust. Traffic around his feet changes his delivery."}},
 {roles(NR::Qb),"throw on run",{"Move the launch point. He can make the throw before the defense gets back to him.","Keep the first moving throws short. I'd see how he handles the harder ones before opening that up.","Let him reset before we ask for the throw back across his body."}},
 {roles(NR::Qb),"throw on run",{"The keeper can still be a pass with him. Don't take the second option away.","Can work a half-field read on the move when the path is clear.","I wouldn't ask him to throw across his body on a rollout. Give him a clean stop first."}},
 {noteBacks,"ball carrier vision",{"He'll take the first good lane instead of waiting for a perfect one. I like that on a tight track.","Reads a defined front well enough. Give him one clear place to start the read.","Too often the lane is gone by the time he picks it. I'd simplify the read before adding options."}},
 {noteBacks,"ball carrier vision",{"I'd give him a chance on runs with a cutback. He sees the backside open without forcing it.","I'd start him behind a clear lead. The crowded fronts still need a longer look.","He can run himself out of the block. Make the aiming point clear and work from there."}},
 {roles(NR::Halfback),"catching",{"Run the empty package with him still out there. The quarterback can use him as an actual target.","He gives us an outlet. I want to see him handle a throw that makes him adjust.","I wouldn't count on an easy catch just because the route is short. Work on the finish first."}},
 {roles(NR::Halfback),"pass block",{"I'd keep him beside the quarterback when we need another body in protection. He can make it count.","Can help on a direct pickup. I'd check the blitzes that change sides on him.","If he's on the field in long yardage, give the protection another answer besides him."}},
 {roles(NR::Fullback),"run block",{"Use him on a short-yardage lead. The back can make a decision behind his block.","Can lead through a defined gap. I want to see him finish when it gets tight.","I'd leave him out of a package that needs him to clear the hole by himself."}},
 {roles(NR::Fullback),"impact blocking",{"Send him across the formation and give him a target. He can make the collision count.","He's workable coming across to block if he has a clear path.","Keep the block close to the snap for now. Finding contact across the formation is a tougher job."}},
 {NR::Receiver|NR::TightEnd,"release",{"He wins the first couple steps with a defender up on him. We can leave him on the line.","Can get into the route against an ordinary jam. I'd give him some help against a bigger corner.","A good jam can take the timing away. Try him in a stack or off the line."}},
 {NR::Receiver|NR::TightEnd,"release",{"We can keep timing routes on the menu against press. He doesn't need a clean runway to get started.","Some releases are cleaner than others. Start with motion when the timing matters most.","I'd give him a free release on the routes that have to be on time."}},
 {roles(NR::Center),"awareness line",{"He sees the front change and can get the guards on the right assignment.","Can handle the usual protection look. I want to see him sort out a twist with the guard.","Put a steady guard next to him. He's going to need help sorting out late movement."}},
 {roles(NR::Edge),"play recognition",{"He can account for the option without giving up the edge chasing the first fake.","Reads a straightforward run. Get a look at him against a puller and a keeper.","I'd give him one clear read on the edge. The second action can draw him inside."}},
 {roles(NR::Interior),"play recognition",{"Reads the block quickly enough to stay in his gap when the back cuts.","Can handle the first block he sees. I'd watch him when the line pulls away.","Keep his gap responsibility clear. He can follow the block and leave the lane behind him."}},
 {roles(NR::Interior),"block shedding",{"He can hold up long enough to make the runner commit, then get free and finish his job.","Can hold his own with a clear read. I want to see him separate when the guard has a head start.","Get him attacking a gap instead of making him sit on the block. He stays occupied too long."}},
 {roles(NR::Edge),"block shedding",{"I'd use him when the edge has to stay firm. He can separate before the runner gets past him.","Can get free with first contact in his favor. Check the reps where the tackle gets there first.","A tackle who lands his hands can keep him out of the play. Needs to clear the block sooner."}},
 {roles(NR::Sam),"block shedding",{"Can meet the tight end and still get back into the run. I like that on the strong side.","Can handle a clear block in front of him. See how he does when it comes from an angle.","Don't put him on the tight end all day. He needs a cleaner way off the block."}},
 {roles(NR::InsideLb),"play recognition",{"He can read the flow without giving up the cutback lane. I'd give him a real look inside.","Gets to the ball when the read stays clean. Show him a couple different backfield looks.","The first fake can take him out of the fit. I'd keep his early assignments simple."}},
 {roles(NR::Will),"pursuit",{"I like him chasing from the backside. He can get there without overrunning the cut.","Can get across when he starts clean. I'd watch how he closes the last few yards.","He gives the runner too much room to cut back. Fix the angle before asking him to chase all the way across."}},
 {noteLbs,"zone coverage",{"Use him underneath when we want to keep the ball in front of the defense. He knows when to pass off a route.","Can take away a short window if he has a clear landmark. Show him a route coming behind the first one.","Keep his first zone small. Receivers can find space behind him when his eyes go to the first route."}},
 {noteLbs,"zone coverage",{"I like him in the short middle. He can read the throw without getting pulled out of his area.","Can play a defined area. I'd give him fewer exchanges until the reads get quicker.","I want a better feel for routes behind him before leaving him in a drop package."}},
 {roles(NR::Corner),"press",{"He can make the receiver work for a clean release and stay with the route after contact.","I'd mix up the picture. He can jam a receiver, but I wouldn't have him live at the line.","Start him in off coverage. If the jam misses, he's working from behind too early."}},
 {roles(NR::FreeSafety),"zone coverage",{"I can leave him deep and trust him to stay over the route without chasing the underneath throw.","Give him one deep landmark first. We can add more exchanges once he's settled.","Don't leave him to cover every deep route from the middle. Start with a smaller piece of the field."}},
 {roles(NR::StrongSafety),"tackle",{"I'd send him downhill. He gets the runner down without selling out for contact.","Can finish in his area. I want more work against a runner with a two-way go.","I'd keep help near him in the run fit. Can't count on the first hit finishing it yet."}},
 {roles(NR::Athlete),"release",{"I'd work him with the receivers. He can handle contact before the route starts.","Receiver is worth exploring. Start him off the line and see where it goes.","I'd hold off on a full move to receiver. Want to see him get off the line with somebody in his face."}},
 {roles(NR::Athlete),"release",{"There's enough at the line to justify a look at an outside receiver spot.","Try him in a stack first if we give him a receiver rep.","If the plan is to try him out wide, start with motion and a cleaner release."}},
};

inline const char* roleLine(NoteRole role,std::string_view key,int band,bool alternate){
 if(alternate&&!oneOfNotes("ball carrier vision|release|long snap|kick power|kick accuracy",key)&&role!=NR::Snapper)return nullptr;
 for(const auto& entry:roleNotes)if((entry.positions&roles(role))&&key==entry.key)return entry.lines[band];
 return nullptr;
}

inline const char* roleVariant(NoteRole role,std::string_view key,int band,uint64_t seed){
 const char* lines[3]={roleLine(role,key,band,false)};
 unsigned count=lines[0]?1u:0u;
 for(const auto& entry:specialistVariants)if(count<3&&(entry.positions&roles(role))&&key==entry.key)lines[count++]=entry.lines[band];
 return count?lines[(seed>>32)%count]:nullptr;
}

inline const char* fieldVariant(NoteRole role,std::string_view key,int band,bool emphasis,uint64_t seed){
 const auto pick=(seed>>35)%4;
 const char* base=roleLine(role,key,band,!emphasis&&pick==0);
 unsigned matches=0;
 const char* variants[2]{};
 for(const auto& entry:fieldVariants)if(matches<2&&(entry.positions&roles(role))&&key==entry.key)variants[matches++]=entry.lines[band];
 if(matches&&pick>=2)return variants[(pick-2)%matches];
 return base;
}

inline std::string_view staffFollowUp(NoteRole role,uint64_t seed){
 using R=NoteRole;
 if(seed%3!=0)return {};
 const unsigned index=(seed>>21)%8;
 if(index>=4){
  switch(role){
  case R::Qb:{constexpr const char* lines[]={"Find out which throws he was allowed to check out of.","I'd watch two-minute work before we ask him to run it.","See how much the staff asked him to handle after a change at the line.","Ask which part of the offense took him the longest to learn."};return lines[index-4];}
  case R::Halfback:case R::Fullback:{constexpr const char* lines[]={"Look at a few games where they asked him to do more than one job.","Ask where the staff used him when they wanted to keep the defense from subbing.","Get a closer look on third down before we settle his role.","Want to see how he handles a run when the front changes his track."};return lines[index-4];}
  case R::Receiver:case R::TightEnd:{constexpr const char* lines[]={"Ask whether they taught him more than one receiver spot.","Check how much the offense asked him to change his routes against pressure.","I'd watch a few ordinary series before we judge him off the catches.","Find out which outside and inside assignments he feels best about."};return lines[index-4];}
  case R::Tackle:case R::Guard:case R::Center:{constexpr const char* lines[]={"Ask what they gave him to handle on obvious passing downs.","Want to see how he sets when the quarterback changes the launch point.","Find out how often they moved him around the line.","Get a look at the short-yardage and two-minute work before we settle on his job."};return lines[index-4];}
  case R::Edge:case R::Interior:{constexpr const char* lines[]={"Find out how much freedom the front gave him to attack a gap.","Want one look at a long drive before we set his workload.","Get a few looks against different kinds of blocking before we give him a spot.","Ask whether he stayed on the field when the offense went to hurry-up."};return lines[index-4];}
  case R::InsideLb:case R::OutsideLb:case R::Sam:case R::Will:{constexpr const char* lines[]={"Find out whether they moved him around based on the offense's personnel.","I want to see what they asked him to do in the red zone.","Ask which coverage jobs he handled when the front brought pressure.","Get a few reps against a back who can run and catch so we see both jobs."};return lines[index-4];}
  case R::Corner:case R::FreeSafety:case R::StrongSafety:{constexpr const char* lines[]={"Ask which receivers the staff chose to match him with.","Get another look at him when the offense goes fast and can't change personnel.","Want to see what his job was when a receiver went in motion.","Ask how often they brought him down closer to the line."};return lines[index-4];}
  case R::Kicker:{constexpr const char* lines[]={"Get a look at him from both hashes before we set his range.","Ask where the staff drew the line between the kick and keeping the offense out.","Want to see him after a long stretch on the sideline.","Get the routine kicks as well as the long ones in the workout."};return lines[index-4];}
  case R::Punter:{constexpr const char* lines[]={"Get a few reps with a short field and a few from our own end.","Ask what his coaches wanted from him when the returner was the problem.","Get him working off a snap that moves him before we settle the job.","I'd see what kind of coverage call he likes for his usual punt."};return lines[index-4];}
  case R::Snapper:{constexpr const char* lines[]={"Put him with both a holder and a punter. I'd like to see each job.","Ask which of the two units he spent more time with.","Want to see the snap on a short clock before we hand him the job.","Get his timing with the regular unit and the backups."};return lines[index-4];}
  case R::Athlete:{constexpr const char* lines[]={"Get his input on which job feels most natural before we move him around.","Ask what the staff would have done with another year to work with him.","Find out how much time he had to learn each side in practice.","Give him more than one kind of rep when we bring him in."};return lines[index-4];}
  default:return {};
  }
 }
 switch(role){
 case R::Qb:{constexpr const char* lines[]={"Ask how much of the protection he set himself.","I'd like to see him take us through a throw he wants back.","Get some red-zone work. Want to see what he does with less room.","Find out how often his offense gave him a choice at the line."};return lines[index];}
 case R::Halfback:case R::Fullback:{constexpr const char* lines[]={"Ask which runs his offense trusted him with when it needed one yard.","Get his take on how the offense handled late pressure.","Want a look at what he gives the offense without the ball.","Find out how much of the passing game he was asked to handle."};return lines[index];}
 case R::Receiver:case R::TightEnd:{constexpr const char* lines[]={"Ask where his coaches liked to line him up when they needed a catch.","Want more reps with him away from the ball.","Find out how much the staff moved him around before the snap.","Get a look at how he and the quarterback handled a route adjustment."};return lines[index];}
 case R::Tackle:case R::Guard:case R::Center:{constexpr const char* lines[]={"Ask who made the calls when the front changed late.","I'd watch him next to a different lineman. Want to see how he works through the exchanges.","Get a few reps against a front that moves after the snap.","Want to see what carries over when he has to work from the other side."};return lines[index];}
 case R::Edge:case R::Interior:{constexpr const char* lines[]={"Find out where his coaches liked to put him on passing downs.","I want some early-down reps before we decide how much to ask of him.","Get a look at him when the offense runs away from his side.","Ask where he'd rather line up when the offense can go either way."};return lines[index];}
 case R::InsideLb:case R::OutsideLb:case R::Sam:case R::Will:{constexpr const char* lines[]={"Ask how his defense handled motion and who made the adjustment call.","Want to see the reads when the back shows him something different after the snap.","Find out whether he stayed in when the offense spread the field.","I'd watch him with less help from the front before settling on a spot."};return lines[index];}
 case R::Corner:case R::FreeSafety:case R::StrongSafety:{constexpr const char* lines[]={"Ask what they trusted him with when the offense changed its formation late.","Get reps with and without help over the top.","I want to see him play the next snap after the ball goes his way.","Find out how often the staff asked him to follow a receiver across the formation."};return lines[index];}
 case R::Kicker:case R::Punter:case R::Snapper:{constexpr const char* lines[]={"I'd get another look with a different operation around him.","Ask what his coaches did when the unit had to get set in a hurry.","Want a full session from warmup to finish before settling who gets the job.","I'd bring him in with the rest of the unit. See how quickly they find the timing."};return lines[index];}
 case R::Athlete:{constexpr const char* lines[]={"Ask which position he wants the first look at.","I'd have two position coaches watch the same reps before we settle on a room.","Find out which responsibilities his coaches trusted him with on each side of the ball.","Leave the position open until we've seen him handle a couple different jobs."};return lines[index];}
 default:return {};
 }
}
}
