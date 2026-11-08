#pragma once
#include "ReactionPalette.h"
#include "NativeSkillSignals.h"
#include "DefensiveMotion.h"
#include "LiveFumble.h"
#include "CatchKnockout.h"
#include "Showboating.h"
#include "ReturnAtmosphere.h"
namespace attendance::awaysound {
struct Outcome {int type=0;bool scoring=false;};
struct Samples {int selectedType=-1,holderSide=-2;float worldZ=NAN;live::ForcedEvent forced;bool fumble=false;
 fumblecontact::Proof contact;knockout::Sample knockout;defense::Sample breakup;showboating::Sample showboat;};
// Reuse the exact trackers over the same verified samples, with symmetric
// classification only. Never feed this mirrored classification into attendance or home energy.
class Model {
 energy::Tracker tracker_;live::Model live_;skills::Model skills_;energy::Outcome result_;
 live::Returns returns_;live::Fumbles fumbles_;knockout::Model knockout_;defense::Model defense_;defensivemotion::Model stops_;showboating::Model showboat_;
 core::Model celebration_;live::ReturnAtmosphere returnSound_;int extraType_=0;
 live::Outcome motion_;live::RunBuildup buildup_;bool taunt_=false;uint64_t motionSerial_=0,motionSequence_=0;
 uint64_t sequence_=0;bool emitted_=false,scored_=false;
public:
 void reset(){*this=Model{};}
 int extraType()const{return extraType_;}
 uint64_t celebration(const core::Settings& c,const energy::Play& p,uint64_t now,float fullness,bool admitted,float minimum){
  auto symmetric=p;symmetric.game.neutral=true;
  auto previous=celebration_.resultCheer;
  celebration_.react(c,result_,now/1000.,symmetric.game,admitted,admitted,minimum);
  if(previous!=celebration_.resultCheer)celebration_.resultType=palette::resultType(result_,tracker_.start,p.game,tracker_.sawPassReleased,sequence_,0,false);
  celebration_.update(c,symmetric,now/1000.,fullness,fullness,0,admitted,minimum);
  return celebration_.erupting&&celebration_.winner==1?celebration_.celebration:0;
 }
 sustained::Intent positiveIntent(const core::Settings& c,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t now,float fullness,bool admitted,float minimum){
  celebration(c,p,now,fullness,admitted,minimum);auto symmetric=p;symmetric.game.neutral=true;
  auto out=celebration_.intent(c,symmetric,match,epoch,now,admitted);
  if((celebration_.erupting&&celebration_.winner!=1)||(!celebration_.erupting&&result_.side!=1)){
   out={};out.match=match;out.epoch=epoch;out.updated=now;out.active=admitted;
  }
  return returnSound_.merge(out,now,fullness);
 }
 const live::Outcome& motion()const{return motion_;}
 live::RunBuildup buildup()const{return buildup_;}
 bool taunt()const{return taunt_;}
 const energy::Outcome& result()const{return result_;}
 // Main situational reactions can arrive before the symmetric gain threshold.
 // Cover that play across ticks, while retaining a later verified score upgrade.
 void cover(uint64_t sequence,bool scoring=false){if(sequence_!=sequence){sequence_=sequence;emitted_=scored_=false;}emitted_=true;scored_=scored_||scoring;}
 void track(const energy::Settings& c,const energy::Play& p,uint64_t mainSequence){auto symmetric=p;symmetric.game.neutral=true;if(p.game.phase==6){tracker_.primeOpeningIntro(symmetric);result_={};}else result_=tracker_.observe(c,symmetric);if(result_.accepted)result_.sequence=mainSequence;}
 Outcome observe(const live::Settings& c,const energy::Play& p,uintptr_t match,unsigned epoch,uint64_t sequence,uint64_t now,bool allowed,
  uintptr_t record,int kind,int ball,float catchSpot,float runnerSpot,bool held,bool skillAllowed,const skills::Sample& sample,const Samples& extra=Samples{}){
  if(sequence_!=sequence){sequence_=sequence;emitted_=scored_=false;}
  auto symmetric=p;symmetric.game.neutral=true;
  motion_={};buildup_={};taunt_=false;extraType_=0;
  bool kick=extra.selectedType==22||extra.selectedType==23||extra.selectedType==24;
  auto event=live_.observe(c,symmetric,match,epoch,now,allowed&&!kick,record,kind,ball,catchSpot,runnerSpot,held,
      symmetric.down==4&&lineup::offensivePlay(extra.selectedType),&result_);
  auto skill=skills_.observe(symmetric,match,epoch,now,allowed&&skillAllowed,held,sample);if(!event.id)event=skill;
  auto contact=skill.id&&skill.event==live::Event::HitStick?skill:event;
  auto returned=returns_.observe(symmetric,match,epoch,now,allowed,extra.selectedType,extra.worldZ,held);if(returned.id)event=returned;
  auto fumble=fumbles_.observe(symmetric,match,epoch,now,allowed,extra.forced,kind,ball,held,extra.fumble,extra.contact,extra.holderSide);
  live::applyRecoveryProof(tracker_,result_,fumbles_.fumbler(),fumbles_.recoveredBy());
  if(fumble.id){event=fumble;live_.cancelRunBuildup(true);}
  auto impact=live::completedImpact(result_,symmetric.game,allowed,motionSequence_,59,live_.fourthConverted());if(impact.id&&!event.id)event=impact;
  auto knocked=knockout_.observe(symmetric,match,epoch,now,allowed,extra.knockout,kind,held);if(knocked.id)event=knocked;
  auto deflected=defense_.observe(symmetric,tracker_.start,result_,match,epoch,sequence,now,allowed,extra.breakup,kind,held,event,motionSequence_);if(deflected.id)event=deflected;
  auto stopped=stops_.observe(tracker_.start,symmetric,result_,contact.id&&contact.event==live::Event::HitStick?contact:event,match,epoch,sequence,now,allowed,motionSequence_);
  if(stopped.id)event=stopped;
  auto taunt=showboat_.observe(tracker_.start,symmetric,match,epoch,sequence,now,allowed,held,extra.showboat);
  if(p.offense==1){buildup_=live_.runBuildup();auto returnedBuildup=returns_.buildup();if(returnedBuildup.id)buildup_=returnedBuildup;
   auto show=showboat_.buildup(now);if(show.id&&!buildup_.id){buildup_=show;taunt_=true;}}
  if(skill.id&&skill.side==1&&event.id&&event.event!=skill.event)extraType_=palette::liveType(skill,tracker_.start,p.game,sequence,epoch,true);
  bool stopRoute=defensivemotion::routed(event.event);
  if(event.id&&event.side==1&&live::cameraReaction(event.event)&&(!stopRoute||stopped.id)){
   motion_=event;motion_.id=++motionSerial_;motionSequence_=sequence;
  }
  auto returnedSound=event;if(returnedSound.side!=1)returnedSound={};
  returnSound_.observe(symmetric,match,epoch,now,allowed,returnedSound,held,extra.fumble);
  if(!allowed||p.game.neutral||!sequence||p.simulated||energy::clockManagementContext(p,tracker_.start,sequence)){motion_={};buildup_={};extraType_=0;return {};}
  bool result=result_.accepted&&result_.completed&&result_.sequence==sequence&&result_.side==1;
  bool scoring=result&&(result_.awayPoints>=6||result_.awayPoints==2||result_.awayPoints==3);
  if(result&&palette::positiveOutcome(result_,tracker_.start)&&(!emitted_||(scoring&&!scored_))){
   emitted_=true;scored_=scored_||scoring;return {palette::resultType(result_,tracker_.start,p.game,tracker_.sawPassReleased,sequence,epoch,true),scoring};
  }
  if(event.id&&event.side==1&&(!emitted_||live::accent(event.event)||event.event==live::Event::RunAfterCatch||event.event==live::Event::Interception||event.event==live::Event::FumbleRecovery||event.event==live::Event::ForcedFumble)){emitted_=true;return {palette::liveType(event,tracker_.start,p.game,sequence,epoch,true),false};}
  if(taunt.id&&taunt.side==1)return {catalog::type(25,3),false};
  return {};
 }
};
}
