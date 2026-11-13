#pragma once
#include <cctype>
#include <string>
#include <utility>
namespace recruitingfog::evidence {
// Presentation only: preserve saved provenance, evidence and assessment bands.
inline std::string naturalNotesText(std::string text){
 for(const auto& edit:{std::pair<const char*,const char*>{" (modeled)",""},{" (MODELED)",""},
   {"Shared a role in his modeled previous season.","Shared reps last season."},
   {"Fictional scouting reports; assessments may be mistaken.",""},
   {"Fictional scouting reports. Assessments may be mistaken.",""},
   {"mod-generated previous season","previous season"},{"fictional previous season","previous season"}}){
  size_t at=0;while((at=text.find(edit.first,at))!=std::string::npos){text.replace(at,std::string(edit.first).size(),edit.second);at+=std::string(edit.second).size();}
 }return text;
}
inline std::string presentNote(std::string text,int quality=0){
 static const std::pair<const char*,const char*> edits[]={
 {"Background note: he keeps a notebook of corrections rather than just watching the highlights.","Keeps a notebook of corrections. Wants more than just his own highlights."},
 {"Background note: his coach describes a quiet player who asks detailed questions after practice.","Quiet kid. His coach says he saves his best questions for after practice."},
 {"Background note: the staff's early impression is of a talkative competitor who likes to set the tone.","Talkative competitor who likes to set the tone around the group."},
 {"Development note: he reportedly learned a second position before settling into this role.","Apparently tried a second position before settling here. That experience could help."},
 {"Development note: the camp report emphasizes repeatable technique over his best single rep.","Camp notes favor repeatable technique over one eye-catching rep."},
 {"Camp note: the early account favors his effort; the staff still wants a broader sample.","Effort drew attention at camp. Want to see it over more reps."},
 {"Background note: he reportedly balances football with another school activity. The details are unconfirmed.","Reportedly balances football with another school activity. Sounds like a busy kid."},
 {"Coach's account: he takes correction well, though that impression still needs checking.","His coach says he takes correction well. Worth watching when things get frustrating."},
 {"Development note: the report describes a late start in organized football, rather than years in one system.","Reportedly came to organized football late. Still has room to settle in."},
 {"Camp note: one strong session should not outweigh the rest of the evaluation.","One good camp session is interesting. Want to see him back it up."},
 {"Background note: teammates reportedly lean on his sense of humor when practice gets repetitive.","Teammates reportedly lean on his sense of humor when practice drags."},
 {"Background lead: he reportedly also wrestles. Treat the claimed leverage benefit as something to verify on football film.","Reportedly wrestles too. Want to see that leverage show up on football film."},
 {"Background lead: he reportedly competes in track. No verified meet result is included in this scouting file.","Reportedly runs track as well. Want to see the speed carry over in pads."},
 {"Background lead: his coach mentions another throwing sport. That does not establish football placement under pressure.","His coach mentions another throwing sport. Football placement still needs a closer look."},
 {"Background lead: the report mentions soccer experience. Repeatable football contact still needs its own evaluation.","Reportedly played soccer. Watch whether the clean contact carries over to football."},
 {"Background lead: he reportedly also plays basketball. The staff wants to see how the footwork carries over to football.","Reportedly plays basketball too. The footwork could be useful here."},
 {"Background lead: the report mentions qualifying for a regional track meet. No official result is attached.","Reportedly qualified for a regional track meet. Promising athletic background."},
 {"Background lead: his coach mentions reaching a school weight-room milestone. The reported lift has not been independently checked.","His coach mentions a school weight-room milestone. Sounds strong; keep checking the film."},

 {"The staff would want him winning with anticipation before stretching the field horizontally.","Needs to win with anticipation before we ask him to stretch the field."},
 {"He can work either side when the throw is on time; late throws are a separate question.","Can work either sideline on time. Late throws need another look."},
 {"The underneath placement is workable, though it is not the whole evaluation.","Placement underneath is usable. Want another look beyond the quick game."},
 {"The staff should simplify his early reads while he learns to recognize changing leverage.","Keep his early reads simple while he learns to sort changing coverage."},
 {"There is a usable foundation to his ball security; the finish through contact still matters.","Protects it reasonably well. Want to see the finish through contact."},
 {"The staff wants him to trust the blocking picture rather than pick an exit too early.","Needs to trust his blocks instead of looking for the exit early."},
 {"His upper body can announce the break before his feet finish selling the stem.","Tips the break upstairs before his feet finish selling the route."},
 {"The staff should make sure he and the quarterback see the same adjustment.","Need to get him and the quarterback seeing the same adjustment."},
 {"His first set and his recovery step do not always look like one connected movement.","First set and recovery step still look like two separate jobs."},
 {"The staff wants clearer communication before adding more protection checks.","Want clearer communication before we add more protection checks."},
 {"His hands need to free him sooner instead of letting the blocker choose where the rep ends.","Hands need to get him free sooner. Blockers are steering him."},
 {"The staff should emphasize balance on the approach before asking for a bigger finish.","Get his feet settled on approach before asking for the big hit."},
 {"He needs a clearer key so that backfield movement does not keep pulling him out of position.","Needs a clearer key. Backfield movement can pull him out of position."},
 {"The staff wants his feet underneath him before he commits to the turn.","Needs his feet under him before committing to the turn."},
 {"His setup needs to become more repeatable before the staff chases extra distance.","Settle the setup first. We can chase extra distance after that."},
 {"The staff would put clean contact ahead of stretching his range for now.","Clean contact comes first. Stretching his range can wait."},
 {"The staff should make the sequence around him as repeatable as possible while he settles in.","Keep the sequence around him consistent while he settles into the job."},
 };
 for(const auto& edit:edits){auto at=text.find(edit.first);if(at!=std::string::npos)text.replace(at,std::string(edit.first).size(),edit.second);}
 constexpr const char* prefixes[]={"Film assessment: ","Coach's early assessment: ","Unverified camp impression: ","Scouting observation: ","Background lead: ","Background note: ","Development note: ","Camp note: ","Coach's account: "};
 for(const auto* prefix:prefixes)if(text.rfind(prefix,0)==0){text.erase(0,std::string(prefix).size());break;}
 // Strip legacy category-linked wrappers equally; they must not reveal quality.
 for(const auto* prefix:{"This remains a tentative read. ","So far, ","His coach sees unusually high upside here. "})if(text.rfind(prefix,0)==0){text.erase(0,std::string(prefix).size());break;}
 if(text=="the report mentions qualifying for a regional track meet.")return "Reportedly qualified for a regional track meet.";
 if(text.rfind("Availability note:",0)==0)return {};
 for(const auto* disclaimer:{" No verified meet result is included in this scouting file."," No official result is attached."," The reported lift has not been independently checked."}){auto at=text.find(disclaimer);if(at!=std::string::npos)text.erase(at,std::string(disclaimer).size());}
 if(text=="the report mentions qualifying for a regional track meet.")text="Reportedly qualified for a regional track meet.";
 if(!text.empty())text[0]=static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
 (void)quality;return naturalNotesText(text);
}
}
