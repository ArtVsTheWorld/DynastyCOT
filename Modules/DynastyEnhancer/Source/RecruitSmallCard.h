#pragma once
#include "RecruitInterestResponse.h"
namespace recruitingfog::interest {
// The Prospect List's native side card has a different, lower-case DTO from
// the recruiting board. Alias only that proven schema; setters still operate
// on the original transient values, never the game database.
template<class View>struct SmallCardView {
 View& view;uintptr_t root;
 bool array(uintptr_t at,std::vector<uintptr_t>& out,size_t limit){if(at==root){out={root};return limit>=1;}return view.array(at,out,limit);}
 bool members(uintptr_t at,std::map<std::string,uintptr_t>& out){if(!view.members(at,out))return false;
  const auto alias=[&](const char* target,const char* source){auto i=out.find(source);if(i!=out.end())out[target]=i->second;};
  if(at==root){alias("RecordId","recordKey");alias("FirstName","firstName");alias("LastName","lastName");alias("TopSchools","topSchoolsSubpanelInfo");}
  else if(out.count("teamLogoId")&&out.count("influenceTotal")){alias("TeamId","teamLogoId");alias("TeamName","teamName");alias("InfluenceTotal","influenceTotal");alias("InfluenceNextTotal","infuenceThisWeek");alias("InfluenceDelta","influenceDelta");alias("InfluenceMinDisplayValue","influenceMinDisplayValue");alias("InfluenceMaxDisplayValue","influenceMaxDisplayValue");alias("IsUserTeam","isUserTeam");}
  return true;
 }
 bool number(uintptr_t at,double& value){return view.number(at,value);}bool string(uintptr_t at,std::string& value){return view.string(at,value);}bool boolean(uintptr_t at,bool& value){return view.boolean(at,value);}
};
template<class View,class StringSetter,class BoolSetter,class NumberSetter,class Reorder>
TransformStats transformSmallCard(View& view,uintptr_t root,Mode mode,uint32_t league,StringSetter text,BoolSetter boolean,NumberSetter number,Reorder reorder){
 std::map<std::string,uintptr_t> fields;if(!view.members(root,fields)||!fields.count("recordKey")||!fields.count("firstName")||!fields.count("lastName")||!fields.count("topSchoolsSubpanelInfo"))return {};
 SmallCardView<View> aliases{view,root};
 // Non-integral marker belongs solely to this display DTO. Native calculations
 // retain their original values, and Native/Rank Only restore the full backup.
 return transform(aliases,root,mode,nullptr,league,text,boolean,number,reorder,100.125);
}
}
