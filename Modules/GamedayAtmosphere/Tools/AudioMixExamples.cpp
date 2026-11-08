#include "../Source/AttendanceAudio.h"
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
 if(argc!=2)return 2;std::ofstream out(argv[1]);out<<"mapped_seats,fullness_pct,occupied_positions,vanilla_crossover_pct,muted,beds_db,reactions_db,individual_detail_db,bed_signal_multiplier\n";
 for(uint64_t seats:{6060,20000,30000,61888})for(int f:{0,1,5,10,20,25,30,50,70,90,100}){
  auto m=attendance::balance::mix(f*.01f,seats);
  out<<seats<<','<<f<<','<<seats*f/100<<','<<100*attendance::balance::crossover(seats)<<','<<m.mute<<','<<m.net(1)<<','<<m.net(3)<<','<<m.net(5)<<','<<(m.mute?0:std::pow(10.f,m.net(1)/20))<<'\n';
 }
 if(!out)return 3;std::cout<<"PASS: production audio ranges exported for four venue sizes.\n";
}
