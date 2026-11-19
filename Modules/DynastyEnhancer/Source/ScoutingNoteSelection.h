#pragma once
#include <cstdint>
#include <array>
#include <string_view>
#include "ScoutingRoleNotes.h"

namespace recruitingfog::evidence {
inline bool tackleNotes(std::string_view pos){return pos=="LT"||pos=="RT";}
inline bool focusedNotes(std::string_view pos){return tackleNotes(pos)||pos=="WR"||pos=="TE";}
inline unsigned noteLimit(double progress){return progress>=75?4:progress>=50?3:progress>=25?2:0;}

inline std::string_view noteTopic(std::string_view key){
 if(key=="speed"||key=="acceleration")return "speed";
 if(key=="agility"||key=="change of direction")return "change of direction";
 if(key=="catching"||key=="catch in traffic"||key=="spectacular catch")return "catching";
 if(key=="short route running"||key=="medium route running"||key=="deep route running")return "route running";
 if(key=="pass block"||key=="pass block finesse"||key=="pass block power")return "pass blocking";
 if(key=="run block"||key=="run block finesse"||key=="run block power"||key=="impact blocking")return "run blocking";
 return key;
}

// Only a revealed range entirely beyond a threshold earns emphatic wording.
// A report that misjudged the band keeps its ordinary level of confidence.
inline int noteEmphasis(double low,double high,int observed,int claim){
 if(observed!=claim)return 0;
 if(claim==0)return low>=93?2:low>=88?1:0;
 if(claim==2)return high<=55?2:high<=62?1:0;
 return 0;
}

struct NoteVoice {std::string_view key;const char* strengths[2];const char* concerns[2];};
inline constexpr NoteVoice focusedVoice[]={
 {"pass block",{"I'd trust him on an island. He gives the QB room to work.","Pass protection is the reason I'd take him. Can hold up without constant help."},
  {"I wouldn't leave him alone in protection yet. Plan to give him help.","Hard to trust him in a long-yardage set. Needs time before we count on him."}},
 {"pass block finesse",{"I'd trust his feet against speed off the edge.","Rushers who win with a second move will have to earn it against him."},
  {"Speed off the edge is a real problem for him. I'd keep a chip nearby.","Quick rushers can win his edge before he's set. Can't ask him to handle that alone yet."}},
 {"pass block power",{"He's hard to walk back into the quarterback. That's a big deal for us.","I'd trust him to sit down against a bull rush and keep the pocket clean."},
  {"Power can put him in the quarterback's lap. The anchor has to get better.","I'd send help against a power rusher. He gives up too much ground right now."}},
 {"run block",{"I'd run behind him when we need a yard. He can open a crease.","Put the point of attack on his side. He's a difference maker in the run game."},
  {"I wouldn't make him the point of attack. Too many blocks end before the back gets there.","The run blocking is a real concern. I'd give him simpler assignments early."}},
 {"run block power",{"He's the guy I'd put at the point of attack on a downhill run.","Want him moving bodies in tight quarters. That's a real selling point."},
  {"Don't ask him to move a heavy front off the ball yet.","He's giving up leverage in tight quarters. I wouldn't run straight behind him right now."}},
 {"run block finesse",{"I'd put him on the move. He can get across a defender and seal the lane.","He's a natural fit on the move. Can make the block without a clean angle to start."},
  {"Reach blocks are a tall order right now. Keep him on simpler angles.","I'd keep him out of the tougher blocks in space until his footwork catches up."}},
 {"impact blocking",{"I like him leading through the hole. First contact changes the rep.","Give him a target on the move. He's got the finish we want."},
  {"I wouldn't put him out front on a lead block yet. Needs a better strike on the move.","Needs to bring more through contact before we send him out as the lead blocker."}},
 {"catching",{"I'd go to him on a gotta-have-it down. The hands are that dependable.","If the ball's there, I'd trust him to finish the catch."},
  {"The hands give me real pause. Get him easy looks before we count on him on third down.","I'd want to see him finish the routine catch before we build the passing game around him."}},
 {"catch in traffic",{"I'd throw to him with a defender on his back. He can finish the catch anyway.","I like him over the middle. We don't need him wide open to trust the throw."},
  {"I wouldn't feed him in traffic yet. Give him clean looks and room to finish.","Tight windows worry me. I'd work him away from contact until the finish improves."}},
 {"spectacular catch",{"He gives the quarterback a lot of room to miss. Can save an off-target throw.","I'd give him a chance on a hard ball. He's good at making an adjustment late."},
  {"Keep the throws on frame. I wouldn't count on him to bail out a miss.","Needs a clean target. Off-body catches are going to be a tough ask."}},
 {"short route running",{"I'd put him in the quick game on day one. He makes the short break count.","Get the ball out on time and trust him to create the window underneath."},
  {"His short breaks give defenders too much time. I'd keep him out of the option menu for now.","The quick game needs cleaner breaks than he's giving us right now."}},
 {"medium route running",{"I'd give him the digs and deep outs. He can get separation right where we need it.","I'd trust him to open a window at the sticks. The intermediate breaks are sharp."},
  {"I wouldn't count on him to open a window at the sticks yet. The breaks get rounded off.","The intermediate routes need work. Give him simpler jobs before we ask him to win there."}},
 {"deep route running",{"I'd give him the deep routes with a break in them. He can sell one thing and break to another.","He's a problem on deep stems. I'd give him chances to make the safety choose."},
  {"The longer routes give the DB too much warning. Keep his early assignments simple.","I wouldn't lean on him for a deep route that needs a sharp break yet."}},
 {"speed",{"Give him grass and let him go. He'll make a defense account for him.","We can stretch the field with him. Defenders will need to respect that pace."},
  {"I wouldn't count on him to run away from coverage. Give him a head start with the design.","A footrace isn't the matchup I'd pick for him. He needs to win another way."}},
 {"acceleration",{"Get him moving early. He can eat up a cushion in a hurry.","That first burst is worth building touches around."},
  {"I wouldn't ask him to win with the first step. Use motion to get him going.","He needs time to get up to speed. Give him a moving start when we can."}},
};

inline const char* emphaticNote(std::string_view key,std::string_view pos,int band,uint64_t seed){
 if(tackleNotes(pos)&&(key=="speed"||key=="acceleration"))return nullptr;
 for(const auto& voice:focusedVoice)if(key==voice.key)return (band==0?voice.strengths:voice.concerns)[(seed>>30)&1];
 return nullptr;
}

inline const char* roleNote(std::string_view key,std::string_view pos,int band,int emphasis,uint64_t seed){
 if(key!="run block")return nullptr;
 if(pos=="TE"){
  if(emphasis)return band==0?"I'd run at his side with him attached. He can handle a tough edge block.":"I wouldn't leave him alone on the edge. Flex him out and give that block to someone else.";
  constexpr const char* edge[3][3]={
   {"I'd keep him attached on runs to his side. He can hold the edge.","I like what he gives us next to the tackle. We can run out of that look.","He's useful as the extra man on the line. Can keep the lane clean."},
   {"Can help next to the tackle. I'd keep the early assignments simple.","I can use him to seal the edge. Still want to see him stick on the block longer.","There's enough here to play attached. Give him a clear angle to start."},
   {"I'd flex him out before asking him to hold the edge by himself.","He's more useful releasing into a route right now. I'd ease him into the blocking work.","The in-line work needs time. Give the tougher edge blocks to someone else for now."},
  };
  return edge[band][(seed>>24)%3];
 }
 if(pos=="WR"){
  if(emphasis)return band==0?"I'd put him in front of our best runner on a screen. He can clear the way.":"Don't ask him to clear the way on screens yet. I'd rather put the ball in his hands.";
  constexpr const char* perimeter[3][3]={
   {"I like him out front on screens. He'll give the ball carrier a lane.","I'd leave him out there when we run to the perimeter. He'll do his part.","He's useful away from the ball. Can keep a corner out of the run fit."},
   {"Can help on a screen when the angle is clean. I'd start there.","Has enough to get in the way on outside runs. Want a better finish.","I'd give him one clear target on perimeter blocks and build from there."},
   {"I'd think twice before making him the lead blocker on a screen.","He gives away too much on the perimeter. Get him the ball instead of asking him to clear the way.","Needs to stay on the corner longer. I wouldn't build our outside run game around his block."},
  };
  return perimeter[band][(seed>>24)%3];
 }
 return nullptr;
}

struct CoupledNote {
 bool tackle;
 const char* first;
 const char* second;
 const char* lines[4]; // both strong, both concerns, first strong, second strong
};
inline constexpr CoupledNote coupledNotes[]={
 {true,"pass block finesse","pass block power",{
  "I can leave him on an island against speed or power. That's a tackle I want.",
  "Quick rushers can beat his edge and power can walk him back. Give him help early.",
  "I trust his feet against speed. The bull rush is the one I'd send help for.",
  "He can sit down on power. I'd still give him a chip against pure speed."}},
 {true,"run block power","run block finesse",{
  "We can run behind him downhill or put him on the move. He fits either job.",
  "Don't make him the lead blocker yet. He's struggling with power and with reach blocks.",
  "I'd run downhill behind him. Asking him to reach and seal is a tougher fit.",
  "Get him on the move. I like his angles more than his ability to move a heavy front."}},
 {false,"catching","catch in traffic",{
  "I'd trust him on third down. Can finish the routine ball and the one with a defender draped over him.",
  "The easy catches aren't settled and traffic makes it worse. I'd start with clean targets.",
  "I trust the hands in space. I'd think twice before sending him into traffic for a must-have catch.",
  "He's better at fighting for a contested ball than his routine catches suggest. Make him clean up the easy ones."}},
 {false,"short route running","medium route running",{
  "I'd use him from the quick game out to the sticks. The break gets him open at both depths.",
  "The short and intermediate breaks need work. Give him a simpler route menu early.",
  "Get him touches underneath. I'd wait before making him our answer at the sticks.",
  "I like him at the sticks. The quick-game breaks need to get sharper."}},
 {false,"speed","acceleration",{
  "Give him room to run. He closes the cushion fast and has another gear once he's through it.",
  "We have to get him open with the call. I don't see the burst or the long speed to do it alone.",
  "He can pull away once he's rolling. Use motion to get him that running start.",
  "He's quick out of the gate. I'd put the ball in his hands early and skip the footrace downfield."}},
};

inline std::string_view roleTopic(NoteRole role,std::string_view key){
 if(oneOfNotes("short accuracy|medium accuracy|deep accuracy",key))return "pass placement";
 if(oneOfNotes("power moves|finesse moves",key))return "pass rush";
 if(oneOfNotes("break tackle|trucking",key))return "contact balance";
 if(role==NR::Athlete&&oneOfNotes("man coverage|zone coverage",key))return "coverage";
 return noteTopic(key);
}

struct RolePair {unsigned positions;const char* first;const char* second;const char* lines[4];};
inline constexpr RolePair rolePairs[]={
 {roles(NR::Qb),"short accuracy","deep accuracy",{
  "I'd trust him underneath and take the shot when it's there. The placement holds up at both ends.",
  "I'd keep the first throws simple. We need cleaner placement underneath before we start asking for deep shots.",
  "I like him in the quick game. I'd be careful counting on the deep ball.",
  "I'd give him chances downfield. The easy ball underneath still needs to get easier."}},
 {NR::Edge|NR::Interior,"power moves","finesse moves",{
  "He has two ways to win the rush. If the blocker sits on power, he can go around him.",
  "The rush needs work across the board. He isn't moving blockers or getting clean around them yet.",
  "I'd let him rush through the blocker. The counter move is the part I'd work on first.",
  "I like the quick move more than the power. Get him a chance to win on an angle."}},
 {noteBacks,"break tackle","trucking",{
  "I'd give him the hard carries. He can run through a tackle or find another way out of it.",
  "Give him some room to work. First contact stops too many of his runs.",
  "He can get out of a tackle without running somebody over. I'd use that in space.",
  "Let him get his pads downhill. I trust the direct finish more than his ability to wriggle free."}},
 {roles(NR::Athlete),"short accuracy","deep accuracy",{
  "I'd give him a real look at quarterback. He can place the short ball and give us something downfield.",
  "I'm not ready to put him in the quarterback room. The placement needs work at both depths.",
  "If we try him at quarterback, start with quick throws. The deeper ball needs time.",
  "There's a downfield arm to work with. I'd want to see the routine throws before settling on quarterback."}},
 {roles(NR::Athlete),"catching","catch in traffic",{
  "I'd give him a look at receiver. He's got starter-level hands in space and he can finish in traffic.",
  "I'd keep other positions open. The hands aren't a reason to move him to receiver right now.",
  "He's worth a look at receiver in space. Don't ask him to make his living in traffic yet.",
  "He's intriguing at the catch point. I'd make him show us the easy catches before settling him at receiver."}},
 {roles(NR::Athlete),"man coverage","zone coverage",{
  "Get him some reps in the secondary. He can stay with a man and has a feel for his space in zone.",
  "I'd keep the secondary as an open question. He's got work to do in both kinds of coverage.",
  "I'd try him in a matchup before handing him a zone to read. The man work is further along.",
  "There's enough feel to try him in a zone. I'd hold off on asking him to live in man coverage."}},
};
}
