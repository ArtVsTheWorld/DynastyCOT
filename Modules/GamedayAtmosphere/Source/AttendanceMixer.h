#pragma once
#include "ChantMixerRuntime.h"
#include "SwellAudioRuntime.h"
#include "AttendanceAudio.h"
namespace attendance::balance {
using namespace chants;
struct Preset {
 std::array<std::unique_ptr<chants::Preset>,8> entries;Array<8*0x28> groups;
 Preset(const std::array<Templates,8>& t,const Mix& mix,uint64_t serial){
  groups.count=8;
  for(unsigned i=0;i<8;++i){entries[i]=std::make_unique<chants::Preset>(t[i],mix.gain[i],serial*8+i,mix.mute&&i==0);std::memcpy(groups.data.data()+i*0x28,entries[i]->groups.data.data(),0x28);}
  entries[0]->preset.put(0x28,groups.address());
 }
 Address address(){return entries[0]->asset.address();}
};
// One native preset synchronizes every parent/child gain and fade. Owned,
// immutable storage outlives native transitions. No graph-volume writes.
class Runtime {
 Address image_=0,manager_=0,handle_=0,asset_=0;
 Create create_=nullptr;Release release_=nullptr;Manager source_=nullptr;Clock clock_=nullptr;
 chants::Runtime reader_;std::array<Templates,8> templatesData_;bool templates_=false;
 std::map<std::array<int,9>,std::unique_ptr<Preset>> presets_;std::mutex mutex_;
 std::atomic<bool> healthy_{true},muted_{false};std::array<std::atomic<float>,7> net_{};std::atomic<float> parent_{0};
 Mix current_;uint64_t changed_=0;std::atomic<uint64_t> creates_{0},releases_{0};
 bool create(Address asset,Address& out){__try{out=create_(manager_,asset);return out!=0;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
 bool release(){__try{release_(manager_,handle_);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
 void clear(){parent_=0;muted_=false;for(auto& db:net_)db=0;current_=Mix{};}
public:
 void configure(Address image,Create make=nullptr,Release end=nullptr,Manager source=nullptr,Clock clock=nullptr){image_=image;create_=make?make:reinterpret_cast<Create>(image+bindings::MixerCreate);release_=end?end:reinterpret_cast<Release>(image+bindings::MixerRelease);source_=source;clock_=clock;reader_.configure(image);}
 void tick(Address owner,bool active,const Mix& proposed){
  if(owner<0x10040||sustained::at<Address>(owner-0x40)!=image_+0xb227098||sustained::at<Address>(owner)!=image_+0xb227178||sustained::at<Address>(owner+8)!=image_+0xb227188)return;
  std::lock_guard<std::mutex> guard(mutex_);Address manager=source_?source_():sustained::at<Address>(image_+bindings::MixerManager);
  if(manager!=manager_){manager_=manager;handle_=asset_=0;clear();templates_=false;changed_=0;}
  if(!manager_)return;Mix wanted=healthy_&&active&&proposed.valid?proposed:Mix{};bool needed=wanted.needed();
  if(handle_&&needed&&wanted.key==current_.key)return;
  auto now=clock_?clock_():GetTickCount64();
  // Zero mute and context loss retire immediately. Other level changes do
  // not replace a preset while its quarter-second transition is running.
  if(handle_&&needed&&!wanted.mute&&now>=changed_&&now-changed_<300)return;
  if(handle_){if(sustained::at<Address>(handle_)!=image_+0xaf01718||(sustained::at<Address>(handle_+8)&~Address(4))!=asset_||!release()){healthy_=false;return;}++releases_;handle_=asset_=0;clear();}
  if(!needed)return;
  if(!templates_){for(unsigned i=0;i<8;++i){if(!reader_.snapshot(manager_,Groups[i],templatesData_[i]))return;if(templatesData_[i].parent!=(i?9:2)||std::strcmp(templatesData_[i].name,Names[i]))return;}templates_=true;}
  auto found=presets_.find(wanted.key);if(found==presets_.end())found=presets_.emplace(wanted.key,std::make_unique<Preset>(templatesData_,wanted,static_cast<uint64_t>(presets_.size()+1))).first;
  Address asset=found->second->address(),handle=0;
  if(!create(asset,handle)||sustained::at<Address>(handle)!=image_+0xaf01718||(sustained::at<Address>(handle+8)&~Address(4))!=asset){healthy_=false;return;}
  handle_=handle;asset_=asset;current_=wanted;parent_=wanted.gain[0];muted_=wanted.mute;
  for(unsigned i=0;i<7;++i)net_[i]=wanted.net(i);changed_=now;++creates_;
 }
 float db()const{return parent_;}float netDb(unsigned i)const{return i<7?net_[i].load():0;}
 float chantDb()const{return netDb(2);}bool muted()const{return muted_;}bool healthy()const{return healthy_;}
 uint64_t creations()const{return creates_;}uint64_t releases()const{return releases_;}
 void fail(){healthy_=false;} // Keep callbacks alive to retire an owned preset.
};
}
