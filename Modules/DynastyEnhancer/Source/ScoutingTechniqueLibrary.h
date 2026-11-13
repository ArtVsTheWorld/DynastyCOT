#pragma once
namespace recruitingfog::evidence {
struct Technique {const char* roles;const char* attribute;const char* bands[3];};
inline constexpr Technique techniques[]={
 {"|QB|","throw power",{"The far-side out stays in the playbook without asking the receiver to wait.","Can work either sideline on time. Late throws need another look.","Needs to win with anticipation before we ask him to stretch the field."}},
 {"|QB|","short accuracy",{"He gives the quick-game receiver a useful ball without forcing a stop and reset.","Placement underneath is usable. Want another look beyond the quick game.","Even the shorter concepts need closer attention to his base and release point."}},
 {"|QB|","awareness qb",{"He appears comfortable moving off the first answer without abandoning the concept.","The first read looks clearer than the picture after the defense rotates.","Keep his early reads simple while he learns to sort changing coverage."}},
 {"|HB|RB|FB|","carrying",{"The ball stays protected as his shoulders turn through traffic.","Protects it reasonably well. Want to see the finish through contact.","The staff wants the ball tucked away before he starts looking for extra yards."}},
 {"|HB|RB|FB|","agility",{"He can change his aiming point without needing a wide loop around the cut.","One clean cut suits him better than stringing several moves together.","His feet need more room than the hole is always going to offer."}},
 {"|HB|RB|FB|","awareness back",{"He shows patience without letting the crease disappear in front of him.","He can follow the designed track; sorting out a crowded front is the next step.","Needs to trust his blocks instead of looking for the exit early."}},
 {"|WR|TE|","catching",{"His hands meet the ball without a lot of extra movement before the tuck.","The routine target is within his comfort zone; repeatability is still the question.","He needs to finish the catch before shifting his attention to the defender."}},
 {"|WR|TE|","short route running",{"He makes the break look like part of the route rather than a separate stop.","The basic breaks are usable, with room to disguise the change of direction.","Tips the break upstairs before his feet finish selling the route."}},
 {"|WR|TE|","awareness receiver",{"He appears to understand where the open space will be, not just where the route was drawn.","The route landmarks look clearer than the adjustments against changing coverage.","Need to get him and the quarterback seeing the same adjustment."}},
 {"|LT|LG|C|RG|RT|","pass block",{"He can stay square long enough to make the rusher work for a second answer.","A stable first set gives him a chance; recovery is still worth watching.","First set and recovery step still look like two separate jobs."}},
 {"|LT|LG|C|RG|RT|","strength",{"There is an anchor to build around when his feet and hands arrive together.","He has enough base for ordinary contact, with technique deciding the rest.","He will need leverage and a better base before the staff relies on him to absorb power."}},
 {"|LT|LG|C|RG|RT|","awareness line",{"He seems ready to pass off a threat rather than chase the first jersey he sees.","The straightforward assignment looks manageable; exchanges need another look.","Want clearer communication before we add more protection checks."}},
 {"|LE|RE|LEDG|REDG|DT|SAM|MIKE|WILL|LOLB|MLB|ROLB|","block shedding",{"He can clear the block without turning his shoulders so far that he loses the ball.","He has a workable escape when the blocker does not land perfectly.","Hands need to get him free sooner. Blockers are steering him."}},
 {"|LE|RE|LEDG|REDG|DT|SAM|MIKE|WILL|LOLB|MLB|ROLB|","tackle",{"He can bring his feet with him instead of reaching and hoping the contact finishes it.","There is a functional tackling base when he arrives under control.","Get his feet settled on approach before asking for the big hit."}},
 {"|LE|RE|LEDG|REDG|DT|SAM|MIKE|WILL|LOLB|MLB|ROLB|","play recognition",{"He appears able to sort the backfield action without giving away his leverage.","The initial key is usable; a second action can still ask him a harder question.","Needs a clearer key. Backfield movement can pull him out of position."}},
 {"|CB|FS|SS|","man coverage",{"His hips and feet let him stay connected without constantly reaching for the receiver.","He has a chance with a clean release; sharper breaks deserve a closer look.","He can lose his balance trying to mirror the receiver's first move."}},
 {"|CB|FS|SS|","zone coverage",{"He seems comfortable protecting his area while keeping the quarterback in view.","The basic spacing is workable, with exchanges still part of the evaluation.","He needs to recognize when a route is leaving his area and when it is becoming his problem."}},
 {"|CB|FS|SS|","agility",{"He can open and redirect without turning every transition into a recovery run.","There is enough movement for the basic transition; sudden counters test the rest.","Needs his feet under him before committing to the turn."}},
 {"|K|P|LS|","kick accuracy",{"The plant and strike appear repeatable rather than depending on a perfect setup.","The basic contact is workable; the staff still wants a broader sample of placement.","Settle the setup first. We can chase extra distance after that."}},
 {"|K|P|LS|","kick power",{"He can create useful distance without appearing to force every swing.","There is a serviceable leg when the contact is clean.","Clean contact comes first. Stretching his range can wait."}},
 {"|K|P|LS|","awareness specialist",{"He appears attentive to the sequence around him rather than treating the strike as an isolated action.","The routine setup is familiar; unusual timing still needs checking.","Keep the sequence around him consistent while he settles into the job."}}
};
}
#include "ScoutingVariety.h"
namespace recruitingfog::evidence {
inline const char* technique(const std::string& position,const std::string& attribute,int band,uint64_t seed){
 if(attribute=="catching"&&(position=="CB"||position=="FS"||position=="SS")){
  constexpr const char* defenseHands[]={"He can finish a chance at the ball instead of only getting a hand to it.","He has a workable chance when the ball reaches his hands cleanly.","Getting into position does not always turn into a clean catch."};return defenseHands[band];
 }
 if(seed%5==0)return nullptr;const auto role="|"+position+"|";
 std::vector<const char*> eligible;
 for(const auto& t:techniques)if(attribute==t.attribute&&std::string(t.roles).find(role)!=std::string::npos)eligible.push_back(t.bands[band]);
 for(const auto& t:additionalTechniques)if(attribute==t.attribute&&std::string(t.roles).find(role)!=std::string::npos)eligible.push_back(t.bands[band]);
 return eligible.empty()?nullptr:eligible[(seed>>29)%eligible.size()];
}
}
