#pragma once
#include "StartingAttendance.h"
namespace attendance::visitors {
enum class AppearanceStage : unsigned {Context,NoFixture,Ownership,Disabled,Plan,NativePlan,Generation,Prepared,Exception,Count};
inline const char* appearanceStageName(AppearanceStage stage){
 switch(stage){
 case AppearanceStage::Context:return "context-disabled/empty/restore-fault";
 case AppearanceStage::NoFixture:return "no-published-fixture";
 case AppearanceStage::Ownership:return "fixture-participants-mismatch";
 case AppearanceStage::Disabled:return "visitor-appearance-disabled";
 case AppearanceStage::Plan:return "supporter-plan-ineligible";
 case AppearanceStage::NativePlan:return "saved/native-layout-preserved";
 case AppearanceStage::Generation:return "native-component/geometry/selectors-unavailable";
 case AppearanceStage::Prepared:return "prepared-before-native-generation";
 case AppearanceStage::Exception:return "exception-selectors-restored";
 default:return "unknown";
 }
}
inline AppearanceStage appearanceAdmission(const turnout::CrowdFixture& fixture,int home,int away,bool enabled){
 if(!fixture.verified)return AppearanceStage::NoFixture;
 if(!fixture.owns(home,away))return AppearanceStage::Ownership;
 if(!enabled&&!fixture.seed.spring)return AppearanceStage::Disabled;
 return AppearanceStage::Prepared; // Admitted; plan/component validation still follows.
}
}
