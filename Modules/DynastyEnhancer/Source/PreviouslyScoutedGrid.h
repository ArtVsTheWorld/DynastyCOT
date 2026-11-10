#pragma once
#include "RecruitClassGrid.h"
namespace recruitingfog::features {
inline uintptr_t rowKeyAddress=0,rowKeyTableAddress=0;
inline std::atomic<unsigned> scoutingIconWrites{0};
struct GridSink {void** table;void* target;struct Text{unsigned row;std::string key,value;};std::vector<Text> text;};
template<class T>inline void gridPut(GridSink* s,unsigned row,const char* key,T value,unsigned slot){reinterpret_cast<void(__fastcall*)(void*,unsigned,const char*,T)>((*reinterpret_cast<void***>(s->target))[slot])(s->target,row,key,value);}
inline void __fastcall gridReserve(GridSink* s,unsigned n){reinterpret_cast<void(__fastcall*)(void*,unsigned)>((*reinterpret_cast<void***>(s->target))[1])(s->target,n);}
inline void __fastcall gridBool(GridSink* s,unsigned r,const char* k,bool v){gridPut(s,r,k,v,3);}
inline void __fastcall gridInt(GridSink* s,unsigned r,const char* k,int v){gridPut(s,r,k,v,4);}
inline void __fastcall gridReal(GridSink* s,unsigned r,const char* k,float v){gridPut(s,r,k,v,5);}
inline bool scoutingTarget(const char* v){return v&&(!strcmp(v,"TargetNone")||!strcmp(v,"TargetLockedOut")||!strcmp(v,"TargetDealbreaker")||!strcmp(v,"TargetInterested"));}
inline void __fastcall gridText(GridSink* s,unsigned r,const char* k,const char* v){if(s->text.size()<64&&k&&!strcmp(k,"display")&&scoutingTarget(v))s->text.push_back({r,k,v?v:""});else gridPut(s,r,k,v,6);}
inline void* gridTable[]={nullptr,reinterpret_cast<void*>(gridReserve),nullptr,reinterpret_cast<void*>(gridBool),reinterpret_cast<void*>(gridInt),reinterpret_cast<void*>(gridReal),reinterpret_cast<void*>(gridText)};
inline bool nameMatches(const std::string& display,const interest::Knowledge::Seen& known){if(known.first.empty()||known.last.empty())return false;const auto initial=known.first.substr(0,1);return display==known.first+" "+known.last||display==initial+"."+known.last||display==initial+". "+known.last;}
inline void gridFlush(GridSink& sink,uintptr_t image,void* source,int form,int row){
 // Only candidate Target cells perform a key lookup. Names, ratings, sort
 // values, active-board/visit states and committed-team logos pass straight
 // through without reading a key or entering the native source again.
 if(sink.text.empty())return;std::optional<unsigned> record;auto data=std::atomic_load(&published);auto fresh=std::atomic_load(&freshScouting);const bool rendererReady=targetUi.retainedDefinitionCount()!=0;
 if(enhancements::presentationAllowed()&&data&&data->userTeam>=0&&!failed[6]&&rowKeyAddress&&rowKeyTableAddress&&*reinterpret_cast<const uintptr_t*>(rowKeyTableAddress)==rowKeyAddress){
  ClassKeySink key{classKeyTable,{}};reinterpret_cast<void(__fastcall*)(void*,void*,int,int)>(rowKeyAddress)(source,&key,form,row);
  if(!key.value.empty()&&key.value.size()<32){std::array<unsigned char,32> bytes{};memcpy(bytes.data(),key.value.data(),key.value.size());ClassKeyBlob blob{32,0,bytes.data(),false};record=reinterpret_cast<unsigned(__fastcall*)(void*)>(image+0x376f6d0)(&blob);}
 }
 for(const auto& item:sink.text){const auto percent=record?interest::currentScouting(*data,fresh,enhancements::currentOptionsLeague.load(),*record,data->userTeam,item.value):std::nullopt;
  // Native TargetNone safely ignores this suffix even before the renderer's
  // first SetData. Do not require a second page load to prime that renderer.
  // Preserve existing lock/dealbreaker glyphs until their renderer is verified.
  const auto display=percent&&*percent>=10&&(rendererReady||item.value=="TargetNone")?"TargetNone,BTFScouted,"+std::to_string(*percent/10*10):item.value;if(display!=item.value)++scoutingIconWrites;gridPut(&sink,item.row,item.key.c_str(),display.c_str(),6);
 }sink.text.clear();
}
}
