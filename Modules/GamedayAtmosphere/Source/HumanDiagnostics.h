#pragma once
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
namespace attendance::readable {
inline std::string trim(std::string value){auto a=value.find_first_not_of(" \t\r\n");if(a==std::string::npos)return {};auto b=value.find_last_not_of(" \t\r\n");return value.substr(a,b-a+1);}
inline std::map<std::string,std::string> fields(const std::string& text){std::map<std::string,std::string> out;std::istringstream input(text);std::string line;while(std::getline(input,line)){auto colon=line.find(':');if(colon!=std::string::npos)out[trim(line.substr(0,colon))]=trim(line.substr(colon+1));}return out;}
inline std::string timing(const std::string& text){
 std::istringstream input(text);unsigned long long calls=0;std::string unit;input>>calls>>unit;auto max=text.find("max ");if(max==std::string::npos)return text;
 double micro=0;std::istringstream(text.substr(max+4))>>micro;std::ostringstream out;out<<calls<<" calls; slowest "<<std::fixed<<std::setprecision(2)<<micro/1000<<" ms";
 auto slow=text.find(">=16ms ");if(slow!=std::string::npos){unsigned long long count=0;std::istringstream(text.substr(slow+7))>>count;out<<"; "<<count<<" took 16 ms or longer";}return out.str();
}
inline std::string status(const std::string& technical){
 auto f=fields(technical);auto get=[&](const char* key){auto i=f.find(key);return i==f.end()?std::string("not available"):i->second;};
 std::ostringstream out;auto first=technical.find('\n');out<<technical.substr(0,first)<<"\n";
 auto line=[&](const char* label,const std::string& value){out<<std::left<<std::setw(29)<<(std::string(label)+":")<<' '<<value<<'\n';};
 line("Updated",get("Updated"));line("Game process ID",get("Game process ID"));
 if(f.count("Activation")){line("Activation",get("Activation"));line("Reason",get("Reason"));out<<"\nDetails: technical.log and technical-status.txt in this folder.\n";return out.str();}
 auto game=get("Game");if(get("Presentation phase")=="7"){auto score=game.find("Home ");game="Final"+(score==std::string::npos?std::string():" | "+game.substr(score));}line("Game",game);line("Teams (home / away IDs)",get("Team IDs (home/away)"));line("Venue",get("Venue"));line("Weather",get("Weather"));
 out<<"\nATTENDANCE\n";line("Attendance profile",get("Attendance profile"));line("Starting attendance",get("Starting attendance"));line("Audio stadium fullness",get("Audio stadium fullness"));line("EA / DGA planned fans",get("EA / revised planned attendance"));if(f.count("Audience foundations"))line("Audience foundations",get("Audience foundations"));line("Crowd vs planned peak",get("Crowd measured"));line("Still arriving",get("Still arriving"));line("Permanent departures",get("Permanent departures"));line("Halftime temporary absence",get("Halftime seat break"));line("2020 Mode",get("2020 Mode"));if(get("2020 Mode")!="off"){line("Collective / band mutes",get("2020 collective crowd / away band / home band mute"));line("Drum cadence mute",get("2020 drum cadence mute"));line("PA announcer mute",get("2020 PA announcer mute"));line("Individual detail gain",get("2020 individual detail gain"));line("Ambience / reverb return",get("2020 ambience / reverb return"));}
 out<<"\nSOUND AND CAMERA\n";line("Opening-drive allowance",get("Opening-drive allowance"));line("Play-call atmosphere",get("Play-call atmosphere"));line("Packed-house threshold",get("Packed-house fullness needed"));line("Upset Watch",get("Upset Watch sound"));line("Background crowd gain",get("Continuous Beds gain"));line("Crowd reactions gain",get("Collective Reactions gain"));line("Added camera motion",get("Added camera motion"));line("Crowd roar",get("Added crowd roar"));line("Celebration carry",get("Postgame celebration carry"));
 out<<"\nREPLAYS AND DIAGNOSTICS\n";line("Replay crowd",get("Replay crowd"));line("Major-play replay policy",get("Major-play replay sequence"));line("Diagnostic preview",get("Diagnostic preview"));auto health=get("Native audio callbacks");auto h=health.find("mixer ");line("Audio mixer health",h==std::string::npos?health:health.substr(h+6));
 out<<"\nPERFORMANCE (wall-clock measurements, not FPS)\n";line("Crowd update",timing(get("DGA crowd apply")));line("Seat preparation",timing(get("DGA seat preparation")));line("Replay crowd check",timing(get("DGA replay crowd check")));line("Log queue",get("Deferred log queue"));line("Log write failures",get("Log write failures"));
 out<<"\nFiles in this folder:\n  DynamicGamedayAtmosphere.log - readable game events and warnings\n  technical.log - detailed events, proofs, native IDs and replay stages\n  technical-status.txt - full status, timings, bindings and settings\n  launcher.log - MMC startup and runtime-loading problems\n\nAudio/replay requests do not prove heard sound or rendered playback.\n";return out.str();
}
inline std::string event(const std::string& original){
 auto tag=original.find(" | ["),end=tag==std::string::npos?std::string::npos:original.find("] ",tag+4);
 if(end==std::string::npos)return original;
 std::string kind=original.substr(tag+4,end-tag-4),message=trim(original.substr(end+2));
 // Failures remain visible even if the event family is normally technical.
 bool warning=kind.find("fault")!=std::string::npos||kind.find("inactive")!=std::string::npos||kind.find("unavailable")!=std::string::npos||kind=="binding-mismatch"||
  message.find("no native cheer voice registered")!=std::string::npos||message.find("queue dropped")!=std::string::npos;
 if(!warning){
  if(kind=="replay-select"||kind=="defensive-proof"||kind=="pass-flight"||kind=="reaction-audio"||kind=="away-motion")return {};
  if(kind=="play-summary"){
   if(message.find("ended; official result still pending")!=std::string::npos)return {};
   auto cut=message.find("; snap down");if(cut!=std::string::npos)message.resize(cut);
   const std::string zeroPoints="; home/away points 0/0";auto zero=message.find(zeroPoints);if(zero!=std::string::npos)message.erase(zero,zeroPoints.size());
   auto points=message.find("home/away points ");if(points!=std::string::npos)message.replace(points,17,"points added (home/away) ");
   auto none=message.find(": None;");if(none!=std::string::npos)message.replace(none,7,": Play completed;");
  }else if(kind=="roar"){
   if(message.find("Winning")==std::string::npos&&message.find("postgame")==std::string::npos)return {};
  }else if(kind=="starting-attendance"){
   auto cut=message.find(" Support standing");if(cut!=std::string::npos){
    auto weather=message.find("weather policy "),kickoff=message.find("local kickoff ");std::string extra;
    for(auto at:{weather,kickoff})if(at!=std::string::npos){auto until=message.find(';',at);extra+=" "+message.substr(at,until==std::string::npos?std::string::npos:until-at)+";";}
    message=message.substr(0,cut)+extra;
   }
  }else if(kind=="replay"){
   if(message.find("Automatic replay:")==0)message="Automatic replay started; live attendance preserved.";
   else if(message.find("Replay ended;")==0)message="Replay ended; restoring live attendance.";
  }else if(kind=="live-play"){
   auto cut=message.find("; native crowd reaction requested");if(cut!=std::string::npos)message=message.substr(0,cut)+"; crowd sound requested.";
  }
 }
 // Bound only the readable copy. The technical copy retains the entire entry.
 if(message.size()>700)message=message.substr(0,700)+" ... (see technical.log)";
 return original.substr(0,tag)+" | ["+(warning?"warning/"+kind:kind)+"] "+message+"\n";
}
}
