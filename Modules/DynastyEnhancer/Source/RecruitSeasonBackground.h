#pragma once
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>
#include "ScoutingRoleNotes.h"

namespace recruitingfog::resume {
template<class Profile>
std::string seasonBackground(std::string_view position,const Profile& profile,uint64_t seed){
 using evidence::NR;
 const auto role=evidence::noteRole(position);
 const auto& workload=profile.at("role").template get_ref<const std::string&>();
 const auto& stats=profile.at("stats");
 const int games=profile.value("games",0);
 auto stat=[&](const char* key){return stats.value(key,0);};
 auto pick=[&](std::initializer_list<const char*> lines){return std::string(lines.begin()[seed%lines.size()]);};
 if(!games)return pick({"No game work to go on from last season. Get a longer look before we settle on him.","He didn't get into a game last season. We'll need to do more of the evaluation ourselves.","There's no game sample from last season. Give him a chance to show us where he fits."});
 if(profile.contains("modeledSecondary")){
  const auto& second=profile.at("modeledSecondary").at("position").template get_ref<const std::string&>();
  const auto primary=std::string(position);
  switch(seed%3){
   case 0:return "Worked at "+primary+" and "+second+". Give that split some weight when we read the totals.";
   case 1:return "Most of his work was at "+primary+", with another role at "+second+". I'd keep both in mind when we bring him in.";
   default:return "Had jobs at "+primary+" and "+second+" last season. Find out which one he wants to work at first.";
  }
 }
 if(workload=="Reserve")return pick({"Got some game work without a start. I'd want more than the season totals before we make a call.","The game sample is mostly reserve work. Get a look at him handling a bigger share.","Came off the bench when he played. We still need to see what a full game asks of him."});
 if(workload=="Late-season starter")return pick({"Took over later in the season. Start with the games after he got the job.","The starting work came late. I'd look at that stretch on its own.","Played more of the year than he started. Give the later starts more weight in our evaluation."});
 std::vector<const char*> candidates;
 auto add=[&](std::initializer_list<const char*> lines){candidates.insert(candidates.end(),lines.begin(),lines.end());};
 if(workload=="Shared role")add({"He shared the job last year. I'd check what changed when the other guy was out there.","Split the workload. Find out which responsibilities were his and which ones the staff kept separate."});
 switch(role){
 case NR::Qb:
  if(stat("carries")>=games*5)add({"There's enough work as a runner to look at how he fits a ground game as well as the passing game.","Don't stop at the passing numbers. The rushing work is worth a look when we decide what to give him."});
  if(stat("passAttempts")>=games*22)add({"There's a good body of passing work. Look at how he handled the games when they leaned on his arm.","Plenty of throws to work through. I'd pull several games before judging him off his best ones."});
  if(stat("passAttempts")>0)add({"Start with the passing work. I want to see which throws his coaches kept coming back to.","The passing totals give us a place to start. Get a look at him late in a drive."});
  break;
 case NR::Halfback:case NR::Fullback:
  if(stat("receptions")>=games*2)add({"Enough catches to consider a passing-down role. I'd watch those snaps alongside the carries.","The receiving work is worth its own look. Find out how much of it we'd give him here."});
  if(stat("carries")>=games*12)add({"Handled a healthy share of the carries. See what he gives us over a full game before settling on his role.","There's real volume here. I'd look at how he ran late in games as well as early."});
  if(stat("runBlockingSnaps")>=games*10&&stat("runBlockingSnaps")>stat("carries")+stat("receptions"))add({"There's a bigger sample of him blocking than the touch totals suggest. Start there.","Get his work as a blocker alongside the carries. That's a big part of the job he'd have here."});
  if(stat("carries")>0)add({"Watch what he does with the carries he got. I want to see how he works once the first lane closes.","There's enough rushing work to spot a few things. I'd still ask how much of the offense he was expected to handle."});
  break;
 case NR::Receiver:case NR::TightEnd:
  if(stat("runBlockingSnaps")>=games*10)add({"Make time for the blocking work. The receiving line only covers one part of his job.","Look at him attached as well as out on a route. There's a decent sample of both jobs here."});
  if(stat("receptions")>0&&stat("receivingYards")>=18*stat("receptions"))add({"He got good yardage out of his catches. Pull those plays and see how much came before and after the catch.","The yards per catch stand out. Find out where those gains came from before we settle on a role."});
  if(stat("targets")>=games*5)add({"They gave him plenty of chances in the pass game. Start with what he did when the ball kept coming his way.","There's a good receiving sample here. See how he handled the ordinary targets as well as the big ones."});
  if(stat("receptions")>0)add({"Get his snaps away from the ball as well as the catches. I want to see a full picture of his work.","There's some receiving work to judge. Find out where they'd line him up when they needed a first down."});
  break;
 case NR::Tackle:case NR::Guard:case NR::Center:
  if(stat("passProtectionSnaps")>=games*26)add({"There's a long enough protection sample to pull several games. Start with the obvious passing downs.","He got a good amount of pass work. I'd look at how much help he needed in those sets."});
  if(stat("runBlockingSnaps")>=games*30)add({"Plenty of work in the run game. Get the snaps where the ball went away from him too.","The run snaps give us somewhere to start. I'd sort out which blocks we'd ask him to make here."});
  if(stat("passProtectionSnaps")>=games*16&&stat("sacksAllowed")*75<=stat("passProtectionSnaps"))add({"The sack total is low for the amount he protected. Check how the unit handled his side before we give him all the credit.","Good sack numbers for that workload. I'd check the protection alongside the quarterback's timing."});
  add({"Pull both sides of the job. The best run blocks can leave you with questions about protection.","There's work here in both phases. Ask what changed when the front made the line adjust."});
  break;
 case NR::Edge:case NR::Interior:case NR::InsideLb:case NR::OutsideLb:case NR::Sam:case NR::Will:
  if(stat("sacks")>=4)add({"Enough sacks to pull the rushes together. See which ones he'd have a chance to win for us.","The rush production is worth a closer look. Separate what he created from what opened up around him."});
  if(stat("tacklesForLoss")>=games)add({"There's a good sample of him getting involved behind the line. Pull those snaps and see where he started.","He made plays for loss often enough to look at them as a group. See which assignments put him there."});
  if(stat("tackles")>=games*5)add({"He was around enough stops to give us plenty to review. I'd check how he reached the ball.","The tackle workload is useful. See how much came on his side and what he did chasing away from it."});
  add({"Watch the work that never reaches the stat sheet. I want to see what he's doing before the ball finds him.","It's worth looking at a few ordinary downs before we get to his best plays."});
  break;
 case NR::Corner:case NR::FreeSafety:case NR::StrongSafety:
  if(stat("passesDefended")+stat("interceptions")>=5)add({"Enough work on the ball to pull a few different looks. See where he started before the throw.","Get the breakups and picks together. I want to see which chances he made for himself."});
  if(stat("coverageTargets")>0&&stat("coverageTargets")<games*4)add({"The ball didn't go his way much. See what he was doing on the throws that went elsewhere.","There's a smaller target sample. Don't let it be the only thing we judge his coverage on."});
  if(stat("tackles")>=games*4)add({"The tackle work is worth a look alongside coverage. I want to see where those stops came from.","Get his work coming downhill as well as the throws his way. There's enough of both to start."});
  add({"Pull a series at a time when we watch him. The balls his way won't tell us everything he was asked to do.","Find out what help he had in coverage before we settle on how we'd use him."});
  break;
 case NR::Kicker:
  if(stat("fieldGoalAttempts")>=games)add({"There's a decent field-goal sample. I'd sort the kicks by distance before judging his consistency.","He took enough tries for a longer look. See what the staff asked of him from different spots."});
  if(stat("longestFieldGoal")>=47)add({"He showed some range. I still want to see how much we'd trust him near the edge of it.","The long make is worth a look. Put the other tries next to it before we decide on his range."});
  if(stat("extraPointAttempts")>0)add({"Look at the routine kicks alongside the field goals. That's a big part of what we'd give him.","There are enough kicks to start a conversation. I'd watch the operation before handing him the job."});
  break;
 case NR::Punter:
  if(stat("puntsInside20")*4>=stat("punts")&&stat("punts")>0)add({"He's got a real sample of balls that ended up inside the twenty. Check the starting spots before we judge the placement.","Look at the punts that pinned the return team. See how much room he had to work with on each one."});
  if(stat("punts")>=games*3)add({"There's plenty of punt work here. Separate the long-field asks from the chances to pin somebody.","Enough punts to judge more than his longest one. I'd pull a mix of field positions."});
  if(stat("punts")>0)add({"The gross average gives us a start. I still want to see where the ball and coverage arrived.","Check what the return team was left with. Distance doesn't tell us the whole job."});
  break;
 case NR::Snapper:
  if(stat("specialTeamsSnaps")>0&&stat("badSnaps")==0)add({"Clean line on the snap chart. I'd check whether the timing holds up with different units.","The chart is clean. Get him with our group and see if the operation stays that way."});
  if(stat("badSnaps")>0)add({"There are some misses on the chart. Pull them together and see if there's a pattern.","I'd look at the off-line snaps alongside the clean ones. Find out what changed."});
  if(stat("specialTeamsSnaps")>=games*5)add({"A good amount of work as the snapper. Ask how he handled the operation when they had to hurry.","There's enough snapping volume to get a fair look. I want punts and shorter snaps in the same review."});
  if(stat("specialTeamsSnaps")>0)add({"Start with the snap work. See how much adjustment the rest of the unit had to make.","Get a few different games from last season. Check whether the snap stayed the same from one to the next."});
  break;
 default:break;
 }
 if(candidates.empty())return pick({"The season gives us a starting point. I'd want more work before we settle on a role.","Pull a few complete games before making a call on where he'd fit.","Start with the work he was given. Get another look at what we'd ask of him here."});
 return candidates[seed%candidates.size()];
}
}
