using System;using System.IO;using System.Runtime.InteropServices;using System.Globalization;
using DynamicGamedayAtmosphere;
class SettingsTests {
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode,EntryPoint="GetPrivateProfileIntW")]static extern uint NativeProfileInt(string section,string key,int fallback,string path);
 [DllImport("kernel32.dll",CharSet=CharSet.Unicode)]static extern uint GetPrivateProfileString(string section,string key,string fallback,System.Text.StringBuilder output,uint size,string path);
 static uint GetPrivateProfileInt(string section,string key,int fallback,string path){
  string category,name;SettingsMigration.Category(section,key,out category,out name);var output=new System.Text.StringBuilder(4);
  GetPrivateProfileString(category,name,"\u0001",output,4,path);
  return output.ToString()=="\u0001"?NativeProfileInt(section,key,fallback,path):NativeProfileInt(category,name,fallback,path);
 }
 static void Check(bool okay,[System.Runtime.CompilerServices.CallerLineNumber]int line=0){if(!okay)throw new Exception("Settings migration regression at line "+line);}
 static double Level(string wantedSection,string wantedKey,string file){
  string mappedSection,mappedKey;SettingsMigration.Category(wantedSection,wantedKey,out mappedSection,out mappedKey);
  string section="";foreach(string line in File.ReadAllLines(file)){
   string text=line.Trim();if(text.StartsWith("[")&&text.EndsWith("]")){section=text.Substring(1,text.Length-2);continue;}
   int equal=text.IndexOf('=');if(equal<1||!((section==wantedSection&&text.Substring(0,equal)==wantedKey)||(section==mappedSection&&text.Substring(0,equal)==mappedKey)))continue;
   return Double.Parse(text.Substring(equal+1).Split(';','#')[0].Trim(),CultureInfo.InvariantCulture);
  }throw new Exception("Missing percentage setting "+wantedKey);
 }
 static int Main(){
  string dir=Path.Combine(Path.GetTempPath(),"GamedaySettingsTest-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(dir);
  string old=Path.Combine(dir,"old.ini"),next=Path.Combine(dir,"new.ini");
  try{
   string defaults=File.ReadAllText(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"..","..","Source","DynamicGamedayAtmosphere.ini"));
   SettingsMigration.Ensure(next,old,defaults);string fresh=File.ReadAllText(next);
   Check(GetPrivateProfileInt("Dynamic Gameday Broadcast","PauseMenuFullness",0,next)==1);
   Check(GetPrivateProfileInt("Attendance","2020ModeEnabled",99,next)==0);
   Check(Level("Dynamic Gameday Broadcast","HostileOpeningShakeMaximumPercent",next)==10);
   Check(fresh.Contains("Ordinary opening drives use critical-down fullness"));
   Check(Math.Abs(Level("Atmosphere","MaximumBoostPercent",next)-30)<.001&&Math.Abs(Level("Atmosphere","EruptionBoostPercent",next)-50)<.001);
   Check(GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59&&!fresh.Contains("[Energy]")&&!fresh.Contains("Momentum="));
   Check(Level("Dynamic Gameday Broadcast","AwayTeamReactionShakeEnabled",next)==1&&Level("Dynamic Gameday Broadcast","NeutralSiteShakePercent",next)==75);
   Check(!fresh.Contains("MaximumReductionPercent="));
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==45);
   Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==100);
   Check(GetPrivateProfileInt("Audio","CrowdVolumeReductionPercent",0,next)==25);
   Check(Level("Dynamic Gameday Broadcast","PlayReactionFadeSeconds",next)==5&&Level("Dynamic Gameday Attendance","HalftimeDepartureConversionPercent",next)==25);
   Check(GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==50&&!fresh.Contains("\nStrength="));
   Check(Level("Atmosphere","ChantMaximumBoostPercent",next)==15&&!fresh.Contains("ChantReductionPercent=")&&!fresh.Contains("BoostDb=")&&!fresh.Contains("ChantReductionDb="));
   Check(GetPrivateProfileInt("Arrivals","FcsOpponentPenalty",0,next)==0&&GetPrivateProfileInt("Retention","FormulaStrength",0,next)==100);
   Check(GetPrivateProfileInt("Arrivals","ModerateStartPercent",0,next)==0&&GetPrivateProfileInt("Arrivals","FinishPercentOfQuarter",0,next)==50&&GetPrivateProfileInt("Arrivals","LateSchoolFinishPercentOfQuarter",0,next)==75);
   Check(GetPrivateProfileInt("General","AttendanceBalanceVersion",0,next)==9);
   Check(GetPrivateProfileInt("Retention","MaximumDeparturePerQuarter",0,next)==45);
   Check(GetPrivateProfileInt("Replays","Enabled",0,next)==1&&GetPrivateProfileInt("Replays","FullScreenMinimumYards",0,next)==12);
   Check(GetPrivateProfileInt("Replays","InsetAfterRoutinePlays",0,next)==1&&GetPrivateProfileInt("Replays","FullScreenBigPlays",0,next)==1);
   Check(GetPrivateProfileInt("Replays","DefensiveHighlights",0,next)==1&&GetPrivateProfileInt("General","ReplayBalanceVersion",0,next)==1);
   SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==fresh);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=44\nAttendanceBalanceVersion=7\n[Dynamic Gameday Attendance]\n; My arrival note.\nStartPercent=55\nFinishPercentOfQuarter=25\nBigGameStartPercent=60\nBadWeatherPenalty=10\n[Custom]\nKeep=yes\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","FinishPercentOfQuarter",0,next)==50&&GetPrivateProfileInt("Dynamic Gameday Attendance","BigGameStartPercent",0,next)==0);
   Check(File.ReadAllText(next).Contains("My arrival note.")&&File.ReadAllText(next).Contains("Keep=yes"));
   string updated=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==updated);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=44\nAttendanceBalanceVersion=8\n[Dynamic Gameday Attendance]\nStartPercent=55\nFinishPercentOfQuarter=25\n");
   SettingsMigration.Ensure(next,old,defaults);Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","FinishPercentOfQuarter",0,next)==25);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=44\nAttendanceBalanceVersion=8\n[Dynamic Gameday Attendance]\n; My arrival note.\nStartPercent=45\nLateSchoolStartPercent=40\nBigGameBonus=7\nBigGameStartPercent=52\nFcsOpponentPenalty=3\n");
   SettingsMigration.Ensure(next,old,defaults);string tuned183=File.ReadAllText(next);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==45&&GetPrivateProfileInt("Arrivals","BigGameBonus",0,next)==3);
   Check(!tuned183.Contains("BigGameStartPercent=")&&!tuned183.Contains("FcsOpponentPenalty=")&&tuned183.Contains("My arrival note."));
   SettingsMigration.Ensure(next,old,defaults);Check(tuned183==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=44\nAttendanceBalanceVersion=8\n[Dynamic Gameday Attendance]\nStartPercent=52\nLateSchoolStartPercent=47\nBigGameBonus=2\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==52&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==47&&GetPrivateProfileInt("Arrivals","BigGameBonus",0,next)==2);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=44\nAttendanceBalanceVersion=7\n[Dynamic Gameday Attendance]\nStartPercent=43\nFinishPercentOfQuarter=68\n");
   SettingsMigration.Ensure(next,old,defaults);Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==43&&GetPrivateProfileInt("Arrivals","FinishPercentOfQuarter",0,next)==68);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=39\n[Dynamic Gameday Broadcast]\nPauseMenuFullness=0\n");
   SettingsMigration.Ensure(next,old,defaults);Check(GetPrivateProfileInt("Dynamic Gameday Broadcast","PauseMenuFullness",1,next)==0);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=39\n[Dynamic Gameday Soundscape]\n; Ordinary matchups keep their usual packed-house requirement.\nOpeningAtmosphere=0\n");
   SettingsMigration.Ensure(next,old,defaults);string openingHelp=File.ReadAllText(next);
   Check(openingHelp.Contains("Ordinary opening drives use critical-down fullness")&&GetPrivateProfileInt("Atmosphere","OpeningAtmosphere",1,next)==0);
   SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==openingHelp);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=39\n[Dynamic Gameday Broadcast]\n; My custom note.\n; Default 4.5. Touchdowns and eruptions take over using their separate profiles.\nPlayReactionFadeSeconds=6\n; Live runs/returns keep the separate post-whistle fade above.\nPlayImpactExtraSeconds=2\n");
   SettingsMigration.Ensure(next,old,defaults);string shapeHelp=File.ReadAllText(next);
   Check(shapeHelp.Contains("brief hold, then a smooth fade")&&shapeHelp.Contains("; My custom note.")&&Level("Dynamic Gameday Broadcast","PlayReactionFadeSeconds",next)==6&&Level("Dynamic Gameday Broadcast","PlayImpactExtraSeconds",next)==2);
   SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==shapeHelp);
   foreach(int version in new[]{0,3,5,6,7,8}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion="+version+"\n[Energy]\nEnabled=1\nSustainedRoars=1\nPlaySurges=1\nMaximumBoostDb=12\nDecisiveMaximumBoostDb=8\nMomentum=1\n[Audio]\nStrength=17\nCommentaryFollowPercent=40\nCloseGameBoostPercent=15\n[Arrivals]\nStartPercent=93\nLateSchoolStartPercent=43\n[Retention]\nQ4GradualThinning=0\nQ4Gradual28Points=3\n[Custom]\nUnknown=keep\n");
    SettingsMigration.Ensure(next,old,defaults);string migrated=File.ReadAllText(next);
    Check(Math.Abs(Level("Atmosphere","MaximumBoostPercent",next)-30)<.001&&Math.Abs(Level("Atmosphere","EruptionBoostPercent",next)-50)<.001);
    Check(GetPrivateProfileInt("Atmosphere","OpeningAtmosphere",0,next)==1&&GetPrivateProfileInt("Atmosphere","WinningEruptions",0,next)==1);
    Check(GetPrivateProfileInt("Energy","Enabled",99,next)==0&&!migrated.Contains("Momentum=")&&!migrated.Contains("CommentaryFollowPercent="));
    Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==80&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==43);
    Check(GetPrivateProfileInt("Retention","Q4GradualThinning",99,next)==0&&GetPrivateProfileInt("Retention","Q4Gradual28Points",0,next)==3&&migrated.Contains("Unknown=keep"));
    Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==100);
    SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==migrated);
   }
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=8\n[Energy]\nEnabled=0\nSustainedRoars=0\nPlaySurges=0\nMaximumBoostDb=2\nDecisiveMaximumBoostDb=3\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Atmosphere","Enabled",1,next)==0&&GetPrivateProfileInt("Atmosphere","OpeningAtmosphere",1,next)==0&&GetPrivateProfileInt("Atmosphere","WinningEruptions",1,next)==0);
   Check(Math.Abs(Level("Atmosphere","MaximumBoostPercent",next)-30)<.001&&Math.Abs(Level("Atmosphere","EruptionBoostPercent",next)-41.25)<.001);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=9\n[Atmosphere]\nOpeningRoar=0\nMaximumBoostDb=1\nEruptionBoostDb=2\n[Audio]\nStrength=8\n");
   SettingsMigration.Ensure(next,old,defaults);string custom=File.ReadAllText(next);
   Check(GetPrivateProfileInt("Atmosphere","OpeningAtmosphere",1,next)==0&&Math.Abs(Level("Atmosphere","MaximumBoostPercent",next)-12.2)<.001&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",99,next)==11);
   SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==custom);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=25\n[Dynamic Gameday Soundscape]\nMaximumReductionPercent=47\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==47&&!File.ReadAllText(next).Contains("MaximumReductionPercent="));
   string quietOnce=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==quietOnce);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=25\n[Dynamic Gameday Soundscape]\nMaximumReductionPercent=0\nNativeQuietingStrengthPercent=100\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==100); // Explicit native setting wins over the retired test cut.
   foreach(int cut in new[]{0,12,25}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=30\n[Dynamic Gameday Soundscape]\nCrowdVolumeReductionPercent="+cut+"\nNativeQuietingStrengthPercent=80\n");SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("Audio","CrowdVolumeReductionPercent",99,next)==cut&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==80);
    string retained=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==retained);
   }
   foreach(int threshold in new[]{20,35,45,50}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=26\n[Dynamic Gameday Soundscape]\nReductionStartPercent="+threshold+"\nNativeQuietingStrengthPercent=100\n");SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==(threshold==35?50:threshold));
    string once=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==once);
   }
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=27\n[Dynamic Gameday Soundscape]\n; Required fullness scales from 99% at 6,060 positions to 60% at 61,888.\n; Smaller venues use 99%; larger venues use 60%. Empty positions count toward size.\nPackedHouseBuildup=0\nReductionStartPercent=50\n");SettingsMigration.Ensure(next,old,defaults);
   string packedHelp=File.ReadAllText(next);Check(packedHelp.Contains("from 98% at 6,060")&&!packedHelp.Contains("use 99%")&&GetPrivateProfileInt("Atmosphere","PackedHouseBuildup",1,next)==0&&GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==50);
   SettingsMigration.Ensure(next,old,defaults);Check(packedHelp==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=28\n[Dynamic Gameday Soundscape]\n; Changes fade over about six seconds. Strong native reactions keep their excitement.\nNativeQuietingStrengthPercent=80\nReductionStartPercent=50\n");SettingsMigration.Ensure(next,old,defaults);
   string nativeHelp=File.ReadAllText(next);Check(!nativeHelp.Contains("Strong native reactions keep")&&!nativeHelp.Contains("extra sparse-crowd peak trim")&&nativeHelp.Contains("one synchronized native bus mix")&&nativeHelp.Contains("Big games add excitement")&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==80);
   SettingsMigration.Ensure(next,old,defaults);Check(nativeHelp==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=29\n[Dynamic Gameday Broadcast]\nNativeShakeMinimumFullnessPercent=0\n[Dynamic Gameday Soundscape]\nNativeQuietingStrengthPercent=85\nReductionStartPercent=50\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Atmosphere","NativeShakeMinimumFullnessPercent",99,next)==0&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==85&&GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==50);
   string presentationHelp=File.ReadAllText(next);Check(presentationHelp.Contains("Automatic clips follow their recorded crowd")&&presentationHelp.Contains("gameplay home-field advantage stay unchanged"));
   SettingsMigration.Ensure(next,old,defaults);Check(presentationHelp==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=45\nSettingsResetVersion=3\n[Dynamic Gameday Broadcast]\n; Default 50; range 0-100. 0 disables this filter. Fuller stadiums keep native shake.\n; My native camera preference.\nNativeShakeMinimumFullnessPercent=0\n[Dynamic Gameday Soundscape]\nLongGainMinimumYards=15\nMaximumBoostPercent=17.5\n");SettingsMigration.Ensure(next,old,defaults);
   string cameraHelp=File.ReadAllText(next);Check(cameraHelp.Contains("Native Stadium Pulse camera shake follows DGA crowd/context admission")&&!cameraHelp.Contains("Fuller stadiums keep native shake.")&&cameraHelp.Contains("My native camera preference.")&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
   Check(GetPrivateProfileInt("Atmosphere","NativeShakeMinimumFullnessPercent",99,next)==0&&Level("Dynamic Gameday Soundscape","MaximumBoostPercent",next)==17.5&&Level("Dynamic Gameday Soundscape","LongGainMinimumYards",next)==15&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==3);
   SettingsMigration.Ensure(next,old,defaults);Check(cameraHelp==File.ReadAllText(next));
   // Existing Update 3 users get current help once, without another INI reset
   // or a change to their custom quieting/camera values and personal notes.
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=42\nSettingsResetVersion=3\n[Dynamic Gameday Soundscape]\n; Changes fade over about six seconds. High native excitement receives an extra sparse-crowd peak trim.\n; My custom quieting note.\nNativeQuietingStrengthPercent=80\nReductionStartPercent=45\n[Dynamic Gameday Broadcast]\nCannonShakeStrengthPercent=23\n");SettingsMigration.Ensure(next,old,defaults);
   string updated43=File.ReadAllText(next);Check(updated43.Contains("one synchronized native bus mix")&&!updated43.Contains("extra sparse-crowd peak trim")&&updated43.Contains("My custom quieting note.")&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==80&&GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==45&&Level("Dynamic Gameday Broadcast","CannonShakeStrengthPercent",next)==23&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==3);
   SettingsMigration.Ensure(next,old,defaults);Check(updated43==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=43\nSettingsResetVersion=3\n[Dynamic Gameday Attendance]\n; In the last quarter of Q4, fans leave faster when a home comeback looks unlikely.\n; My traffic preference.\nLateTrafficEnabled=0\nLateTrafficExtraQuarterAllowance=7\n[Dynamic Gameday Broadcast]\nCriticalShakeStrengthPercent=18\nCannonShakeStrengthPercent=23\n");SettingsMigration.Ensure(next,old,defaults);
   string updated44=File.ReadAllText(next);Check(updated44.Contains("last third of Q4")&&!updated44.Contains("In the last quarter of Q4, fans leave faster")&&updated44.Contains("My traffic preference.")&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==3);
   Check(Level("Dynamic Gameday Attendance","LateTrafficEnabled",next)==0&&Level("Dynamic Gameday Attendance","LateTrafficExtraQuarterAllowance",next)==7&&GetPrivateProfileInt("Atmosphere","CriticalShakeStrengthPercent",0,next)==18&&Level("Dynamic Gameday Broadcast","CannonShakeStrengthPercent",next)==23);
   SettingsMigration.Ensure(next,old,defaults);Check(updated44==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=99\nAudioBalanceVersion=99\n[Energy]\nMomentum=1\n[Atmosphere]\nMaximumBoostDb=12\n[Arrivals]\nStartPercent=70\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("General","ConfigVersion",0,next)==99&&GetPrivateProfileInt("Atmosphere","MaximumBoostDb",0,next)==12&&GetPrivateProfileInt("Energy","Momentum",0,next)==1&&GetPrivateProfileInt("Arrivals","StartPercent",0,next)==70);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=9\n[Audio]\nStrength=20\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==100&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=10\n[Arrivals]\nStartPercent=60\nFinishPercentOfQuarter=50\nLateSchoolFinishPercentOfQuarter=85\nBigGameBonus=10\n[Audio]\nStrength=35\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","FinishPercentOfQuarter",0,next)==50&&GetPrivateProfileInt("Arrivals","LateSchoolFinishPercentOfQuarter",0,next)==75&&GetPrivateProfileInt("Arrivals","BigGameBonus",0,next)==3);
   string tuned=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(tuned==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAttendanceBalanceVersion=1\nAudioBalanceVersion=10\n[Arrivals]\nStartPercent=70\nLateSchoolStartPercent=50\nModerateStartPercent=60\nPlayoffStartPercent=70\nChampionshipStartPercent=80\n[Audio]\nStrength=35\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==45&&GetPrivateProfileInt("Arrivals","ModerateStartPercent",0,next)==0);
   Check(GetPrivateProfileInt("Arrivals","PlayoffStartPercent",0,next)==0&&GetPrivateProfileInt("Arrivals","ChampionshipStartPercent",0,next)==0&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",0,next)==100);
   Check(GetPrivateProfileInt("General","AttendanceBalanceVersion",0,next)==9&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
   foreach(int cap in new[]{30,22}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAttendanceBalanceVersion=2\nAudioBalanceVersion=11\n[Retention]\nMaximumDeparturePerQuarter="+cap+"\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("Retention","MaximumDeparturePerQuarter",0,next)==(cap==30?45:cap)&&GetPrivateProfileInt("General","AttendanceBalanceVersion",0,next)==9);
    string migratedCap=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(migratedCap==File.ReadAllText(next));
   }
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAttendanceBalanceVersion=99\nAudioBalanceVersion=10\n[Retention]\nMaximumDeparturePerQuarter=30\n[Arrivals]\nStartPercent=60\nFinishPercentOfQuarter=50\n");
   SettingsMigration.Ensure(next,old,defaults);Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==60&&GetPrivateProfileInt("General","AttendanceBalanceVersion",0,next)==99&&GetPrivateProfileInt("Retention","MaximumDeparturePerQuarter",0,next)==30);
   foreach(int maximum in new[]{0,12,40}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=11\n[Audio]\nStrength=35\nMaximumReductionPercent="+maximum+"\nReductionStartPercent=85\n");
    SettingsMigration.Ensure(next,old,defaults);string converted=File.ReadAllText(next);
    Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",99,next)==(maximum==40?100:maximum)&&GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==50&&!converted.Contains("\nStrength="));
    SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==converted);
   }
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=11\n[Audio]\nStrength=0\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",99,next)==0);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=99\n[Audio]\nStrength=35\nMaximumReductionPercent=7\nReductionStartPercent=95\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==99&&GetPrivateProfileInt("Audio","Strength",0,next)==35&&GetPrivateProfileInt("Audio","MaximumReductionPercent",0,next)==7&&GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==95);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=12\n[Audio]\nMaximumReductionPercent=12\nChantReductionDb=12\n[Atmosphere]\nMaximumBoostDb=0\nEruptionBoostDb=3\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","MaximumBoostPercent",next)==0&&Math.Abs(Level("Atmosphere","EruptionBoostPercent",next)-41.25)<.001&&!File.ReadAllText(next).Contains("ChantReductionPercent="));
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=12\n[Audio]\nChantReductionDb=6\nChantReductionPercent=12.5\n[Atmosphere]\nMaximumBoostDb=4\nMaximumBoostPercent=20.25\nEruptionBoostDb=6\nEruptionBoostPercent=42.5\n");
   var culture=CultureInfo.CurrentCulture;try{System.Threading.Thread.CurrentThread.CurrentCulture=CultureInfo.GetCultureInfo("fr-FR");SettingsMigration.Ensure(next,old,defaults);}finally{System.Threading.Thread.CurrentThread.CurrentCulture=culture;}
   Check(Level("Atmosphere","MaximumBoostPercent",next)==20.25&&Level("Atmosphere","EruptionBoostPercent",next)==42.5&&!File.ReadAllText(next).Contains("ChantReductionPercent=")&&!File.ReadAllText(next).Contains("BoostDb="));
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=13\n[Replays]\nEnabled=0\nFullScreenMinimumYards=30\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Replays","Enabled",1,next)==0&&GetPrivateProfileInt("Replays","FullScreenMinimumYards",0,next)==30&&GetPrivateProfileInt("Replays","InsetAfterRoutinePlays",0,next)==1);
   string replaySettings=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==replaySettings);
   foreach(int cutoff in new[]{20,25}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=13\n[Replays]\nFullScreenMinimumYards="+cutoff+"\nDefensiveHighlights=0\n");SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("Replays","FullScreenMinimumYards",0,next)==(cutoff==20?12:cutoff)&&GetPrivateProfileInt("Replays","DefensiveHighlights",1,next)==0);
    Check(GetPrivateProfileInt("General","ReplayBalanceVersion",0,next)==1);string tunedReplays=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==tunedReplays);
   }
   File.WriteAllText(next,"[General]\nConfigVersion=1\nReplayBalanceVersion=99\n[Replays]\nFullScreenMinimumYards=20\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Replays","FullScreenMinimumYards",0,next)==20&&GetPrivateProfileInt("General","ReplayBalanceVersion",0,next)==99);
   File.Delete(next);File.WriteAllText(old,"[Attendance]\nEnabled=0\nProfile=BadWeather\n[Audio]\nStrength=17\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Attendance","Enabled",1,next)==0&&GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",99,next)==100&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
   foreach(int strength in new[]{0,8,10}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAttendanceBalanceVersion=3\nAudioBalanceVersion=13\n[Arrivals]\nStartPercent=65\nModerateStartPercent=55\nLateSchoolStartPercent=45\nHalftimeSeatBreakPercent="+strength+"\n[Retention]\nComebackOriginalDeficit=28\nMaximumDeparturePerQuarter=22\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","ModerateStartPercent",0,next)==0&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==45);
    Check(GetPrivateProfileInt("Arrivals","HalftimeSeatBreakStrength",999,next)==strength*10&&!File.ReadAllText(next).Contains("HalftimeSeatBreakPercent="));
    Check(GetPrivateProfileInt("Retention","MaximumDeparturePerQuarter",0,next)==22&&GetPrivateProfileInt("Retention","ComebackOriginalDeficit",0,next)==21&&GetPrivateProfileInt("Retention","MaximumDeparturePerTransition",0,next)==4);
    string balanced=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==balanced);
   }
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=13\nAttendanceBalanceVersion=4\n[Atmosphere]\nMaximumBoostPercent=12.2\nCameraShake=0\n[Custom]\nUnknown=keep\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Atmosphere","CameraShake",1,next)==0&&GetPrivateProfileInt("Atmosphere","PlayCallAtmosphere",0,next)==1&&GetPrivateProfileInt("Atmosphere","ChantMaximumBoostPercent",0,next)==15);
   Check(Math.Abs(Level("Atmosphere","MaximumBoostPercent",next)-12.2)<.001&&File.ReadAllText(next).Contains("Unknown=keep"));
   string lineup=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==lineup);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=14\nAttendanceBalanceVersion=4\n[Atmosphere]\nMaximumBoostPercent=50\nEruptionBoostPercent=70\n[Custom]\nUntouched=yes\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","MaximumBoostPercent",next)==30&&Level("Atmosphere","HighLeverageBoostPercent",next)==35&&Level("Atmosphere","EruptionBoostPercent",next)==50);
   Check(File.ReadAllText(next).Contains("Untouched=yes"));string tiers=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(File.ReadAllText(next)==tiers);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=14\n[Atmosphere]\nEnabled=0\nMaximumBoostPercent=10\nEruptionBoostPercent=20\nHighLeverageBoostPercent=15\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","MaximumBoostPercent",next)==10&&Level("Atmosphere","HighLeverageBoostPercent",next)==15&&Level("Atmosphere","EruptionBoostPercent",next)==20&&GetPrivateProfileInt("Atmosphere","Enabled",1,next)==0);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=15\nAttendanceBalanceVersion=4\n[Arrivals]\nStartPercent=70\nModerateStartPercent=60\nLateSchoolStartPercent=50\n[Atmosphere]\nPackedHouseBuildup=0\nDriveEscalation=0\nMaximumBoostPercent=17.5\nEruptionBoostPercent=42.5\nHighLeverageBoostPercent=26.5\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55&&GetPrivateProfileInt("Arrivals","ModerateStartPercent",0,next)==0&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==45);
   Check(GetPrivateProfileInt("Atmosphere","PackedHouseBuildup",1,next)==0&&GetPrivateProfileInt("Atmosphere","DriveEscalation",1,next)==0);
   Check(Level("Atmosphere","MaximumBoostPercent",next)==17.5&&Level("Atmosphere","EruptionBoostPercent",next)==42.5&&Level("Atmosphere","HighLeverageBoostPercent",next)==26.5);
   string capacity=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(capacity==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=15\nAttendanceBalanceVersion=4\n[Arrivals]\nStartPercent=93\nModerateStartPercent=52\nLateSchoolStartPercent=20\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Arrivals","StartPercent",0,next)==80&&GetPrivateProfileInt("Arrivals","ModerateStartPercent",0,next)==0&&GetPrivateProfileInt("Arrivals","LateSchoolStartPercent",0,next)==30);
   foreach(int cut in new[]{0,12,20}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=17\n[Audio]\nMaximumReductionPercent="+cut+"\nReductionStartPercent=40\n[Atmosphere]\nCameraShake=0\nMaximumBoostPercent=17.5\n[Custom]\nUnknown=keep\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59&&GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==50);
    Check(GetPrivateProfileInt("Audio","NativeQuietingStrengthPercent",999,next)==(cut==20?100:cut));
    Check(GetPrivateProfileInt("Atmosphere","CameraShake",1,next)==0&&Level("Atmosphere","MaximumBoostPercent",next)==17.5&&File.ReadAllText(next).Contains("Unknown=keep"));
    string curve=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(curve==File.ReadAllText(next));
   }
   foreach(double chant in new[]{0.0,7.5,10.0,12.5}){
    File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=18\n[Atmosphere]\n; Starts at no boost at 85% fullness, then rises in a straight line to this limit.\nChantMaximumBoostPercent="+chant.ToString(CultureInfo.InvariantCulture)+"\nCameraShake=0\n[Custom]\n; Keep my custom explanation.\nUntouched=abc\n");
    SettingsMigration.Ensure(next,old,defaults);string documented=File.ReadAllText(next);
    Check(Level("Atmosphere","ChantMaximumBoostPercent",next)==(chant==10?15:chant)&&GetPrivateProfileInt("Atmosphere","BigGameMinimumFullnessPercent",0,next)==55);
    Check(GetPrivateProfileInt("Atmosphere","CameraShake",1,next)==0&&documented.Contains("Keep my custom explanation.")&&documented.Contains("Untouched=abc"));
    Check(!documented.Contains("at 85% fullness, then rises")&&documented.Contains("no independent chant boost/cut")&&documented.Contains("Optional extra floor for big-game buildup"));
    SettingsMigration.Ensure(next,old,defaults);Check(documented==File.ReadAllText(next));
   }
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=19\n[Atmosphere]\n; My current help.\nChantMaximumBoostPercent=10\nBigGameMinimumFullnessPercent=67\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","ChantMaximumBoostPercent",next)==10&&GetPrivateProfileInt("Atmosphere","BigGameMinimumFullnessPercent",0,next)==67&&File.ReadAllText(next).Contains("My current help."));
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=19\n[Atmosphere]\n; My shake switch.\nCameraShake=0\nOrdinaryShakeStrengthPercent=8\n[Custom]\nUnknown=keep\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Atmosphere","CameraShake",1,next)==0&&Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==8);
   Check(Level("Atmosphere","CriticalShakeStrengthPercent",next)==10&&Level("Atmosphere","TouchdownShakeStrengthPercent",next)==16&&Level("Atmosphere","EruptionShakeStrengthPercent",next)==22.5);
   Check(File.ReadAllText(next).Contains("My shake switch.")&&File.ReadAllText(next).Contains("Sustained eruption movement")&&File.ReadAllText(next).Contains("Unknown=keep"));
   string motion=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(motion==File.ReadAllText(next));
   Check(Level("Atmosphere","TouchdownShakeImpactPercent",next)==20&&Level("Atmosphere","EruptionShakeImpactPercent",next)==30);
   Check(Level("Atmosphere","TouchdownShakeDurationSeconds",next)==11&&Level("Atmosphere","EruptionShakeDurationSeconds",next)==16);
   File.WriteAllText(next,"[General]\nConfigVersion=1\nAudioBalanceVersion=19\n[Atmosphere]\n; My strength note.\nOrdinaryShakeStrengthPercent=7\nTouchdownShakeImpactPercent=24\nEruptionShakeImpactPercent=35\nTouchdownShakeDurationSeconds=12\nEruptionShakeDurationSeconds=20\n; Quick rise, then roughly three seconds of settling. Eruptions take priority.\nTouchdownShakeStrengthPercent=15\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==7&&Level("Atmosphere","TouchdownShakeImpactPercent",next)==24&&Level("Atmosphere","EruptionShakeImpactPercent",next)==35);
   Check(Level("Atmosphere","TouchdownShakeDurationSeconds",next)==12&&Level("Atmosphere","EruptionShakeDurationSeconds",next)==20&&File.ReadAllText(next).Contains("My strength note."));
   Check(!File.ReadAllText(next).Contains("roughly three seconds")&&File.ReadAllText(next).Contains("level after the opening hit"));
   string impacts=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(impacts==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=19\n[Dynamic Gameday Attendance]\nEnabled=0\nArrivalsEnabled=0\n[Dynamic Gameday Soundscape]\nMaximumBoostPercent=12.5\nLayeredAtmosphereEnabled=0\nLongCompletionMinimumYards=22\n[Dynamic Gameday Broadcast]\nCriticalShakeStrengthPercent=8\nDuringPlayShakeStrengthPercent=13\n[Atmosphere]\nMaximumBoostPercent=25\n[Custom]\n; Keep my note.\nUnknown=xyz\n");
   SettingsMigration.Ensure(next,old,defaults);string categories=File.ReadAllText(next);
   Check(GetPrivateProfileInt("General","ConfigVersion",0,next)==2&&GetPrivateProfileInt("Attendance","Enabled",1,next)==0&&GetPrivateProfileInt("Arrivals","Enabled",1,next)==0);
   Check(Level("Atmosphere","MaximumBoostPercent",next)==12.5&&GetPrivateProfileInt("Atmosphere","Enabled",1,next)==0);
   Check(GetPrivateProfileInt("Dynamic Gameday Soundscape","LongCompletionMinimumYards",0,next)==22&&GetPrivateProfileInt("Dynamic Gameday Broadcast","DuringPlayShakeStrengthPercent",0,next)==13);
   Check(categories.Contains("[Dynamic Gameday Attendance]")&&categories.Contains("[Dynamic Gameday Soundscape]")&&categories.Contains("[Dynamic Gameday Broadcast]")&&!categories.Contains("[Atmosphere]"));
   Check(categories.Contains("Keep my note.")&&categories.Contains("Unknown=xyz"));SettingsMigration.Ensure(next,old,defaults);Check(categories==File.ReadAllText(next));
   foreach(int cutoff in new[]{15,22,30}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=19\n[Dynamic Gameday Soundscape]\nLongCompletionMinimumYards="+cutoff+"\n");SettingsMigration.Ensure(next,old,defaults);
    Check(GetPrivateProfileInt("Dynamic Gameday Soundscape","LongCompletionMinimumYards",0,next)==(cutoff==15||cutoff==30?20:cutoff));
    Check(GetPrivateProfileInt("Dynamic Gameday Soundscape","LongGainMinimumYards",0,next)==15);string revised=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(revised==File.ReadAllText(next));
   }
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=20\nAttendanceBalanceVersion=5\n[Dynamic Gameday Attendance]\nStartPercent=60\nModerateStartPercent=50\nModerateFinishPercentOfQuarter=50\nPlayoffStartPercent=65\nChampionshipStartPercent=75\n[Dynamic Gameday Soundscape]\nLongGainMinimumYards=30\n[Dynamic Gameday Broadcast]\nEruptionShakeStrengthPercent=30\nEruptionShakeImpactPercent=40\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Dynamic Gameday Attendance","BigGameStartPercent",0,next)==0&&GetPrivateProfileInt("Arrivals","StartPercent",0,next)==55);
   Check(!File.ReadAllText(next).Contains("ModerateStartPercent=")&&!File.ReadAllText(next).Contains("PlayoffStartPercent="));
   Check(Level("Atmosphere","EruptionShakeStrengthPercent",next)==22.5&&Level("Atmosphere","EruptionShakeImpactPercent",next)==30&&GetPrivateProfileInt("Dynamic Gameday Soundscape","LongGainMinimumYards",0,next)==15);
   string twoGroups=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(twoGroups==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=21\nAttendanceBalanceVersion=6\n[Dynamic Gameday Soundscape]\nReductionStartPercent=25\nBigGameMinimumFullnessPercent=35\n[Dynamic Gameday Broadcast]\n; My custom camera note.\nOrdinaryShakeStrengthPercent=5\nCriticalShakeStrengthPercent=10\nTouchdownShakeDurationSeconds=9\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59&&GetPrivateProfileInt("General","AttendanceBalanceVersion",0,next)==9);
   Check(GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==50&&GetPrivateProfileInt("Atmosphere","BigGameMinimumFullnessPercent",0,next)==55);
   Check(Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==5&&Level("Atmosphere","CriticalShakeStrengthPercent",next)==10&&Level("Atmosphere","TouchdownShakeDurationSeconds",next)==11);
   Check(Level("Dynamic Gameday Broadcast","LineupCheerShakeStrengthPercent",next)==6&&File.ReadAllText(next).Contains("My custom camera note."));
   string calmer=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(calmer==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=21\nAttendanceBalanceVersion=6\n[Dynamic Gameday Soundscape]\nReductionStartPercent=20\n[Dynamic Gameday Broadcast]\nOrdinaryShakeStrengthPercent=0\nCriticalShakeStrengthPercent=8\nTouchdownShakeDurationSeconds=14\nLineupCheerShakeStrengthPercent=0\n");SettingsMigration.Ensure(next,old,defaults);
   Check(GetPrivateProfileInt("Audio","ReductionStartPercent",0,next)==20&&Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==0&&Level("Atmosphere","CriticalShakeStrengthPercent",next)==10);
   Check(Level("Atmosphere","TouchdownShakeDurationSeconds",next)==14&&Level("Dynamic Gameday Broadcast","LineupCheerShakeStrengthPercent",next)==0);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=22\n[Dynamic Gameday Soundscape]\nLongCompletionMinimumYards=30\nLongGainMinimumYards=25\n[Dynamic Gameday Broadcast]\nOrdinaryShakeStrengthPercent=3\nCriticalShakeStrengthPercent=3\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==5&&Level("Atmosphere","CriticalShakeStrengthPercent",next)==10);
   Check(GetPrivateProfileInt("Dynamic Gameday Soundscape","LongCompletionMinimumYards",0,next)==20&&GetPrivateProfileInt("Dynamic Gameday Soundscape","LongGainMinimumYards",0,next)==15&&GetPrivateProfileInt("Dynamic Gameday Soundscape","PassFlightAnticipation",0,next)==1);
   string resized=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(resized==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=22\n[Dynamic Gameday Soundscape]\nLongCompletionMinimumYards=22\nLongGainMinimumYards=18\nPassFlightAnticipation=0\n[Dynamic Gameday Broadcast]\nOrdinaryShakeStrengthPercent=0\nCriticalShakeStrengthPercent=8\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==0&&Level("Atmosphere","CriticalShakeStrengthPercent",next)==10&&GetPrivateProfileInt("Dynamic Gameday Soundscape","LongCompletionMinimumYards",0,next)==22&&GetPrivateProfileInt("Dynamic Gameday Soundscape","LongGainMinimumYards",0,next)==18&&GetPrivateProfileInt("Dynamic Gameday Soundscape","PassFlightAnticipation",1,next)==0);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=24\n[Dynamic Gameday Broadcast]\n; Random motion ceiling on critical downs during packed-house/big-game buildup.\n; Third down needs 7 or fewer yards.\n; My camera note.\nCriticalShakeStrengthPercent=8\nOrdinaryShakeStrengthPercent=0\n");SettingsMigration.Ensure(next,old,defaults);
   string newCameraHelp=File.ReadAllText(next);Check(Level("Atmosphere","CriticalShakeStrengthPercent",next)==10&&Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==0&&newCameraHelp.Contains("10 or fewer yards")&&!newCameraHelp.Contains("7 or fewer yards")&&newCameraHelp.Contains("My camera note."));
   SettingsMigration.Ensure(next,old,defaults);Check(newCameraHelp==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=33\n[Dynamic Gameday Attendance]\nHalftimeDepartureConversionPercent=0\n[Dynamic Gameday Broadcast]\nPlayReactionFadeSeconds=6\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Attendance","HalftimeDepartureConversionPercent",next)==0&&Level("Dynamic Gameday Broadcast","PlayReactionFadeSeconds",next)==6);
   string latest=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(latest==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=34\n[Dynamic Gameday Soundscape]\nLongGainMinimumYards=20\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Soundscape","LongGainMinimumYards",next)==15&&Level("Dynamic Gameday Broadcast","PlayImpactExtraSeconds",next)==1);
   string earlier=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(earlier==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=35\n[Dynamic Gameday Soundscape]\nLongGainMinimumYards=20\n[Dynamic Gameday Broadcast]\nPlayImpactExtraSeconds=2\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Soundscape","LongGainMinimumYards",next)==15&&Level("Dynamic Gameday Broadcast","PlayImpactExtraSeconds",next)==2);
   Check(Level("Dynamic Gameday Broadcast","CatchImpactShakeStrengthPercent",next)==12&&Level("Dynamic Gameday Attendance","LateTrafficEnabled",next)==1&&Level("Dynamic Gameday Attendance","LateTurnoverExtraMax",next)==10);
   Check(Level("Dynamic Gameday Broadcast","CannonShakeStrengthPercent",next)==18);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=42\n[Dynamic Gameday Broadcast]\nDuringPlayShakeStrengthPercent=0\nCannonShakeStrengthPercent=7.5\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Broadcast","DuringPlayShakeStrengthPercent",next)==0&&Level("Dynamic Gameday Broadcast","CannonShakeStrengthPercent",next)==7.5);
   string cannonCustom=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(cannonCustom==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=36\n[Dynamic Gameday Broadcast]\nCatchImpactShakeStrengthPercent=7.5\n[Dynamic Gameday Attendance]\nLateTrafficEnabled=0\nLateTurnoverExtraMax=4\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Broadcast","CatchImpactShakeStrengthPercent",next)==7.5&&Level("Dynamic Gameday Attendance","LateTrafficEnabled",next)==0&&Level("Dynamic Gameday Attendance","LateTurnoverExtraMax",next)==4);
   string trafficCustom=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(trafficCustom==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=39\n[Dynamic Gameday Broadcast]\nHostileOpeningShakeMaximumPercent=0\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Broadcast","HostileOpeningShakeMaximumPercent",next)==0);
   string hostileCustom=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(hostileCustom==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=39\n[Dynamic Gameday Soundscape]\n; Retired distance fade.\nReactionDistanceExtraSeconds=8\n[Dynamic Gameday Broadcast]\nPlayReactionFadeSeconds=4.5\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Broadcast","PlayReactionFadeSeconds",next)==5&&!File.ReadAllText(next).Contains("ReactionDistanceExtraSeconds")&&!File.ReadAllText(next).Contains("Retired distance fade"));
   string fixedFade=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(fixedFade==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=40\nAttendanceBalanceVersion=7\n[Dynamic Gameday Attendance]\n; Most fans allowed to leave permanently in one ordinary quarter: 1-50 points\n; of the original crowd. Postseason games also have their own lower limits.\nMaximumDeparturePerQuarter=35\nLateTrafficEnabled=0\nLateTrafficExtraQuarterAllowance=0\n[Dynamic Gameday Soundscape]\nMaximumBoostPercent=17.5\nSkillMoveReactions=0\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Attendance","MaximumDeparturePerQuarter",next)==35&&Level("Dynamic Gameday Attendance","LateTrafficEnabled",next)==0&&Level("Dynamic Gameday Attendance","LateTrafficExtraQuarterAllowance",next)==0);
   Check(Level("Dynamic Gameday Soundscape","MaximumBoostPercent",next)==17.5&&Level("Dynamic Gameday Soundscape","SkillMoveReactions",next)==0&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
   Check(File.ReadAllText(next).Contains("limited extra")&&!File.ReadAllText(next).Contains("; Most fans allowed to leave permanently"));
   string trafficHelp=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(trafficHelp==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=41\n[Dynamic Gameday Soundscape]\nMaximumBoostPercent=17.5\nHighLeverageBoostPercent=25\nEruptionBoostPercent=40\nPackedHouseBuildup=0\nUpsetWatchAtmosphere=0\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Soundscape","UpsetWatchAtmosphere",next)==0&&Level("Dynamic Gameday Soundscape","PackedHouseBuildup",next)==0);
   Check(Level("Dynamic Gameday Soundscape","MaximumBoostPercent",next)==17.5&&Level("Dynamic Gameday Soundscape","HighLeverageBoostPercent",next)==25&&Level("Dynamic Gameday Soundscape","EruptionBoostPercent",next)==40);
   string upsetSettings=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(upsetSettings==File.ReadAllText(next));
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=41\n[Dynamic Gameday Soundscape]\nPlayCallAtmosphere=0\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Soundscape","UpsetWatchAtmosphere",next)==1&&Level("Dynamic Gameday Soundscape","PlayCallAtmosphere",next)==0);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=41\n[Dynamic Gameday Soundscape]\n; My pass-swell note.\n; Reuses the roar/swell; no extra volume bonus, cheer or shake before the result.\nPassFlightAnticipation=0\nDuringPlayReactionsEnabled=0\nMaximumBoostPercent=17.5\nHighLeverageBoostPercent=25\nEruptionBoostPercent=40\n");SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Soundscape","PassFlightAnticipation",next)==0&&Level("Dynamic Gameday Soundscape","DuringPlayReactionsEnabled",next)==0);
   Check(Level("Dynamic Gameday Soundscape","MaximumBoostPercent",next)==17.5&&Level("Dynamic Gameday Soundscape","HighLeverageBoostPercent",next)==25&&Level("Dynamic Gameday Soundscape","EruptionBoostPercent",next)==40);
   string passHelp=File.ReadAllText(next);Check(passHelp.Contains("0.5-second attack")&&passHelp.Contains("My pass-swell note.")&&GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
   SettingsMigration.Ensure(next,old,defaults);Check(passHelp==File.ReadAllText(next));
   foreach(double gain in new[]{25.0,23.25,0.0}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=46\n[Dynamic Gameday Soundscape]\n; My gain choice.\nMaximumBoostPercent="+gain.ToString(CultureInfo.InvariantCulture)+"\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(Level("Dynamic Gameday Soundscape","MaximumBoostPercent",next)==(gain==25?30:gain)&&File.ReadAllText(next).Contains("My gain choice."));
    string stableGain=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(stableGain==File.ReadAllText(next));
   }
   foreach(bool customMotion in new[]{false,true}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=47\n[Dynamic Gameday Broadcast]\n; Keep my motion note.\nOrdinaryShakeStrengthPercent="+(customMotion?"0":"5")+"\nCriticalShakeStrengthPercent="+(customMotion?"7.25":"10")+"\nDuringPlayShakeStrengthPercent="+(customMotion?"0":"10")+"\nCatchImpactShakeStrengthPercent="+(customMotion?"6.5":"8")+"\nHostileOpeningShakeMaximumPercent="+(customMotion?"0":"15")+"\nTouchdownShakeStrengthPercent="+(customMotion?"17.5":"15")+"\nCannonShakeStrengthPercent=9\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(Level("Atmosphere","OrdinaryShakeStrengthPercent",next)==(customMotion?0:5));
    Check(Level("Atmosphere","CriticalShakeStrengthPercent",next)==(customMotion?7.25:10));
    Check(Level("Dynamic Gameday Broadcast","DuringPlayShakeStrengthPercent",next)==(customMotion?0:11));
    Check(Level("Dynamic Gameday Broadcast","CatchImpactShakeStrengthPercent",next)==(customMotion?6.5:12));
    Check(Level("Dynamic Gameday Broadcast","HostileOpeningShakeMaximumPercent",next)==(customMotion?0:10));
    Check(Level("Atmosphere","TouchdownShakeStrengthPercent",next)==(customMotion?17.5:16)&&Level("Dynamic Gameday Broadcast","CannonShakeStrengthPercent",next)==9);
    string stableMotion=File.ReadAllText(next);Check(stableMotion.Contains("Keep my motion note."));
    SettingsMigration.Ensure(next,old,defaults);Check(stableMotion==File.ReadAllText(next));
   }
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=49\nSettingsResetVersion=3\n[Dynamic Gameday Soundscape]\n; My reaction preference.\nSituationalCrowdReactions=0\nMaximumBoostPercent=23.25\n");SettingsMigration.Ensure(next,old,defaults);
   Check(File.ReadAllText(next).Contains("auditioned native families")&&File.ReadAllText(next).Contains("My reaction preference.")&&Level("Dynamic Gameday Soundscape","MaximumBoostPercent",next)==23.25&&Level("Dynamic Gameday Soundscape","SituationalCrowdReactions",next)==0&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==3);
   string audio50=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(audio50==File.ReadAllText(next));
   foreach(bool customLineup in new[]{false,true}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=50\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\n; My buildup choice.\nOrdinaryShakeStrengthPercent="+(customLineup?"0":"4")+"\nCriticalShakeStrengthPercent="+(customLineup?"7.25":"8")+"\nLineupCheerShakeStrengthPercent="+(customLineup?"2.25":"5")+"\nWidePlayerCameraCompensation=1\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(Level("Dynamic Gameday Broadcast","OrdinaryShakeStrengthPercent",next)==(customLineup?0:5));
    Check(Level("Dynamic Gameday Broadcast","CriticalShakeStrengthPercent",next)==(customLineup?7.25:10));
    Check(Level("Dynamic Gameday Broadcast","LineupCheerShakeStrengthPercent",next)==(customLineup?2.25:6));
    Check(Level("Dynamic Gameday Broadcast","WidePlayerCameraCompensation",next)==1&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==4&&File.ReadAllText(next).Contains("My buildup choice."));
    string stable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(stable==File.ReadAllText(next));
   }
   foreach(double cannonLevel in new[]{15.0,0.0,7.5,23.0}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=51\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\n; My cannon note.\nCannonShakeStrengthPercent="+cannonLevel.ToString(CultureInfo.InvariantCulture)+"\n");
    SettingsMigration.Ensure(next,old,defaults);Check(Level("Dynamic Gameday Broadcast","CannonShakeStrengthPercent",next)==(cannonLevel==15?18:cannonLevel)&&File.ReadAllText(next).Contains("My cannon note."));
    Check(File.ReadAllText(next).Contains("fixed 11-to-15-yard buildup")&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==4);
    string cannonStable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(cannonStable==File.ReadAllText(next));
   }
   foreach(int threshold in new[]{20,0,18,25}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=54\nSettingsResetVersion=4\n[Dynamic Gameday Soundscape]\nLongGainMinimumYards="+threshold+"\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(Level("Dynamic Gameday Soundscape","LongGainMinimumYards",next)==(threshold==20?15:threshold));
    Check(GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59);
    string stable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(stable==File.ReadAllText(next));
   }
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=55\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\nZoomCameraShakePercent=125\nStandardCameraShakePercent=0\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Broadcast","ZoomCameraShakePercent",next)==125&&Level("Dynamic Gameday Broadcast","StandardCameraShakePercent",next)==0);
   foreach(string camera in new[]{"WideCameraShakePercent","All22CameraShakePercent","BroadcastCameraShakePercent"})Check(Level("Dynamic Gameday Broadcast",camera,next)==100);
   string cameraStable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(cameraStable==File.ReadAllText(next)&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==4);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=55\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\nStandardCameraShakePercent=110\nZoomCameraShakePercent=130\nAll22CameraShakePercent=75\nWideCameraShakePercent=100\nBroadcastCameraShakePercent=75\nCatchImpactShakeStrengthPercent=9\n[Dynamic Gameday Soundscape]\nCrowdMixStrengthPercent=175\n");
   SettingsMigration.Ensure(next,old,defaults);
   foreach(string camera in new[]{"StandardCameraShakePercent","ZoomCameraShakePercent","All22CameraShakePercent","WideCameraShakePercent","BroadcastCameraShakePercent"})Check(Level("Dynamic Gameday Broadcast",camera,next)==100);
   Check(Level("Dynamic Gameday Broadcast","CatchImpactShakeStrengthPercent",next)==12&&Level("Dynamic Gameday Soundscape","CrowdMixStrengthPercent",next)==100);
   string hotfixStable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(hotfixStable==File.ReadAllText(next));
   foreach(int enabled in new[]{0,1}){
    File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=57\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\nAwayTeamReactionShake="+enabled+"\nZoomCameraShakePercent=125\n");
    SettingsMigration.Ensure(next,old,defaults);
    Check(Level("Dynamic Gameday Broadcast","AwayTeamReactionShake",next)==(enabled==1?35:0));
    Check(Level("Dynamic Gameday Broadcast","NeutralSiteShakePercent",next)==75&&Level("Dynamic Gameday Broadcast","ZoomCameraShakePercent",next)==125);
    string sliderStable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(sliderStable==File.ReadAllText(next));
   }
   // After conversion,1 really means1%; do not repeatedly treat it as On.
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=58\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\nAwayTeamReactionShake=1\nNeutralSiteShakePercent=40\n");
   SettingsMigration.Ensure(next,old,defaults);Check(Level("Dynamic Gameday Broadcast","AwayTeamReactionShake",next)==1&&Level("Dynamic Gameday Broadcast","NeutralSiteShakePercent",next)==40);
   File.WriteAllText(next,"[General]\nConfigVersion=2\nAudioBalanceVersion=58\nSettingsResetVersion=4\n[Dynamic Gameday Broadcast]\n; Keep my camera choices.\nStandardCameraShakePercent=0\nZoomCameraShakePercent=125\nAll22CameraShakePercent=150\nAwayTeamReactionShake=1\nNeutralSiteShakePercent=40\nWidePlayerCameraCompensation=1\n");
   SettingsMigration.Ensure(next,old,defaults);
   Check(Level("Dynamic Gameday Broadcast","StandardCameraShakePercent",next)==0&&Level("Dynamic Gameday Broadcast","ZoomCameraShakePercent",next)==125&&Level("Dynamic Gameday Broadcast","All22CameraShakePercent",next)==150);
   Check(Level("Dynamic Gameday Broadcast","AwayTeamReactionShake",next)==1&&Level("Dynamic Gameday Broadcast","NeutralSiteShakePercent",next)==40&&Level("Dynamic Gameday Broadcast","WidePlayerCameraCompensation",next)==1);
   Check(GetPrivateProfileInt("General","AudioBalanceVersion",0,next)==59&&GetPrivateProfileInt("General","SettingsResetVersion",0,next)==4&&File.ReadAllText(next).Contains("Keep my camera choices."));
   string normalizedStable=File.ReadAllText(next);SettingsMigration.Ensure(next,old,defaults);Check(normalizedStable==File.ReadAllText(next));
   Console.WriteLine("PASS: clean/legacy settings, native quieting/motion migration, custom camera multipliers and missing defaults, comments, idempotence and future schemas.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
  finally{foreach(string file in Directory.GetFiles(dir))File.Delete(file);Directory.Delete(dir);}
 }
}
