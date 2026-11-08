#include "../Source/LogFiles.h"
#include "../Source/LogPolicy.h"
#include "../Source/CallbackTiming.h"
#include "../Source/HumanDiagnostics.h"
#include <cassert>
#include <chrono>
#include <iostream>
static std::string contents(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
 auto folder=std::filesystem::temp_directory_path()/("GamedayLogTest-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 auto path=folder/"Logs"/"atmosphere.log";
 assert(attendance::appendLog(path,"first\n",16)&&contents(path)=="first\n");
 assert(attendance::appendLog(path,"second\n",16)&&contents(path)=="first\nsecond\n");
 assert(attendance::appendLog(path,"new\n",16)&&contents(path)=="new\n");
 assert(attendance::appendLog(path,std::string(40,'X'),16)&&std::filesystem::file_size(path)==16);
 assert(attendance::appendLog(path,"after\n",16)&&contents(path)=="after\n");
 assert(!attendance::appendLog(path,"ignored",0)&&contents(path)=="after\n");
 assert(!std::filesystem::exists(path.string()+".1")&&!std::filesystem::exists(path.string()+".2"));
 auto batchPath=folder/"Logs"/"batch.log",singlePath=folder/"Logs"/"single.log";
 std::vector<std::string> entries={"one\n","two\n",std::string(40,'Y'),"after\n"};
 for(const auto& entry:entries)assert(attendance::appendLog(singlePath,entry,16));
 assert(attendance::appendLogBatch(batchPath,entries,16)&&contents(batchPath)==contents(singlePath));
 auto old=folder/"Logs"/"DynamicGamedayAtmosphere.log";assert(attendance::appendLog(old,"old full detail\n",1024));attendance::prepareReadableLogs(folder/"Logs");
 assert(contents(folder/"Logs"/"technical.log")=="old full detail\n"&&contents(old).empty());
 assert(attendance::appendLog(old,"new readable\n",1024));attendance::prepareReadableLogs(folder/"Logs");assert(contents(old)=="new readable\n");
 auto event=[](const std::string& tag,const std::string& message){return "2026-10-30 14:00:00 | ["+tag+"] "+message+"\n";};
 auto technical=event("play-summary","Play #75: Fourth-down conversion; 3 yards; home/away points 0/0; added reactions eligible; snap down 4; native proof 123");
 auto human=attendance::readable::event(technical);assert(human.find("Fourth-down conversion")!=std::string::npos&&human.find("native proof")==std::string::npos&&technical.find("native proof 123")!=std::string::npos);
 assert(attendance::readable::event(event("play-summary","Play #4 ended; official result still pending.")).empty());
 assert(attendance::readable::event(event("replay-select","native identity0x123 phase4")).empty());
 assert(attendance::readable::event(event("reaction-audio","native cheer voice registered")).empty());
 assert(attendance::readable::event(event("reaction-audio","no native cheer voice registered within the short recovery window")).find("warning/")!=std::string::npos);
 assert(attendance::readable::event(event("audio-inactive","callback unavailable")).find("callback unavailable")!=std::string::npos);
 assert(attendance::readable::event(event("play-summary","Play #4: None; 2 yards; snap down 1;")).find("Play completed")!=std::string::npos);
 std::string status="Dynamic Gameday Atmosphere 2.0.11\nUpdated: 14:00:00\nGame process ID: 42\nAudio stadium fullness: 75.4% of mapped seating\nDiagnostic preview: idle\nDGA crowd apply: 20 calls; max 15000 us\n";
 status+="Attendance profile: Fuller Crowds\n";auto readable=attendance::readable::status(status);assert(readable.find("Fuller Crowds")!=std::string::npos);assert(readable.find("75.4% of mapped seating")!=std::string::npos&&readable.find("technical-status.txt")!=std::string::npos&&readable.find("15.00 ms")!=std::string::npos);
 auto audienceStatus=attendance::readable::status("Dynamic Gameday Atmosphere 2.1.1\nStarting attendance: EA; waiting for fresh launch\nAudience foundations: native field labels verified; awaiting a matching fresh launch read\n");
 assert(audienceStatus.find("EA; waiting for fresh launch")!=std::string::npos&&audienceStatus.find("awaiting a matching fresh launch read")!=std::string::npos);
 auto inactive=attendance::readable::status("Dynamic Gameday Atmosphere 2.0.11\nGame process ID: 42\nActivation: INACTIVE\nReason: signature differs\n");assert(inactive.find("signature differs")!=std::string::npos&&inactive.find("INACTIVE")!=std::string::npos);
 std::filesystem::remove_all(folder/"Logs");
 assert(attendance::appendLog(path,"recreated\n",16)&&contents(path)=="recreated\n");
 auto blocker=folder/"not-a-directory";{std::ofstream f(blocker);f<<"keep";}
 assert(!attendance::appendLog(blocker/"invalid.log","ignored",16)&&contents(blocker)=="keep");
 std::filesystem::remove(blocker);
 std::filesystem::remove(path);std::filesystem::remove(folder/"Logs");std::filesystem::remove(folder);
 for(const char* tag:{"fault","audio-inactive","skill-reactions-unavailable","native-camera-unavailable","2020-mode","defensive-motion","showboat"})assert(attendance::importantLogTag(tag,"test"));
 assert(attendance::importantLogTag("audio","mixer-fault")&&!attendance::importantLogTag("audio","routine callback"));
 assert(!attendance::importantLogTag("entity","routine discovery")&&attendance::importantLogTag("away-motion","applied"));
 attendance::LogQueue queue;queue.push("first");queue.push("second");auto batch=queue.take(1);assert(batch.size()==1&&batch[0]=="first"&&queue.size()==1);
 for(int i=0;i<1000;++i)queue.push(std::to_string(i));assert(queue.size()==512&&queue.dropped()==489);assert(queue.take(512).back()=="999"&&queue.size()==0);
 queue.push(std::string(100000,'X'));assert(queue.take()[0].size()==16384);
 attendance::timing::Counts counts;counts.record(20);counts.record(4000);counts.record(20000);counts.record(10);assert(counts.calls==4&&counts.maximumUs==20000&&counts.over4ms==2&&counts.over16ms==1);
 attendance::DuplicateLogFilter filter;assert(filter.accept("history/fault",1000));assert(filter.accept("audio/fault",1100));assert(!filter.accept("history/fault",1200));assert(filter.accept("history/fault",6000));
 for(int i=0;i<1000;++i)filter.accept(std::to_string(i),7000+i);assert(filter.size()==128);assert(filter.accept("rollback",1));
 std::cout<<"PASS: nested log folder, bounded append, clear-before-overflow, oversized entries, no archives and bounded per-message duplicate suppression across interleaved faults.\n";
}
