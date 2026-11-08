using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Globalization;
namespace DynamicGamedayAtmosphere {
    internal static class SettingsIo {
        internal static void Run(string operation,string source,string destination,string backup,Action action){
            try {
                foreach(string path in new[]{source,destination,backup})
                    if(path!=null&&Path.GetFullPath(path).Length>=260)
                        throw new PathTooLongException("DGA settings need paths shorter than 260 characters for this MMC host.");
                action();
            }catch(Exception error){
                error.Data["DgaSettingsOperation"]=operation;
                if(source!=null)error.Data["DgaSettingsSource"]=source;
                if(destination!=null)error.Data["DgaSettingsDestination"]=destination;
                if(backup!=null)error.Data["DgaSettingsBackup"]=backup;
                if(error is PathTooLongException)error.Data["DgaSettingsHint"]="Use a shorter MMC installation path; existing settings are preserved.";
                throw;
            }
        }
        internal static string ReadText(string path){
            string text=null;Run("read existing settings",path,null,null,()=>text=File.ReadAllText(path));return text;
        }
        internal static void Cleanup(string temp,Exception primary){
            if(!File.Exists(temp))return;
            try{Run("remove temporary settings file",temp,null,null,()=>File.Delete(temp));}
            catch(Exception cleanup){if(primary==null)throw;primary.Data["DgaSettingsCleanup"]="Temporary file: "+temp+" | "+cleanup.GetType().Name+": "+cleanup.Message;}
        }
    }
    public static class SettingsMigration {
        sealed class LayoutKey {public string OldSection,OldKey,Section,Key;public LayoutKey(string os,string ok,string ns,string nk){OldSection=os;OldKey=ok;Section=ns;Key=nk;}}
        static readonly LayoutKey[] Layout={
            new LayoutKey("Attendance","AttendanceProfile","Dynamic Gameday Attendance","AttendanceProfile"),
            new LayoutKey("Attendance","Enabled","Dynamic Gameday Attendance","Enabled"),
            new LayoutKey("Attendance","2020ModeEnabled","Dynamic Gameday Attendance","2020ModeEnabled"),
            new LayoutKey("Attendance","InitialAttendanceEnabled","Dynamic Gameday Attendance","InitialAttendanceEnabled"),
            new LayoutKey("Attendance","Profile","Dynamic Gameday Attendance","Profile"),
            new LayoutKey("Retention","FormulaStrength","Dynamic Gameday Attendance","FormulaStrength"),
            new LayoutKey("Retention","MaximumDeparturePerQuarter","Dynamic Gameday Attendance","MaximumDeparturePerQuarter"),
            new LayoutKey("Retention","MaximumDeparturePerTransition","Dynamic Gameday Attendance","MaximumDeparturePerTransition"),
            new LayoutKey("Retention","ComebackOriginalDeficit","Dynamic Gameday Attendance","ComebackOriginalDeficit"),
            new LayoutKey("Retention","ComebackPointsRecovered","Dynamic Gameday Attendance","ComebackPointsRecovered"),
            new LayoutKey("Retention","ComebackDeteriorationSeconds","Dynamic Gameday Attendance","ComebackDeteriorationSeconds"),
            new LayoutKey("Retention","TwoMinuteWarning","Dynamic Gameday Attendance","TwoMinuteWarning"),
            new LayoutKey("Retention","LateScoreExtra","Dynamic Gameday Attendance","LateScoreExtra"),
            new LayoutKey("Arrivals","Enabled","Dynamic Gameday Attendance","ArrivalsEnabled"),
            new LayoutKey("Arrivals","StartPercent","Dynamic Gameday Attendance","StartPercent"),
            new LayoutKey("Arrivals","LateSchoolStartPercent","Dynamic Gameday Attendance","LateSchoolStartPercent"),
            new LayoutKey("Arrivals","ModerateStartPercent","Dynamic Gameday Attendance","ModerateStartPercent"),
            new LayoutKey("Arrivals","FinishPercentOfQuarter","Dynamic Gameday Attendance","FinishPercentOfQuarter"),
            new LayoutKey("Arrivals","LateSchoolFinishPercentOfQuarter","Dynamic Gameday Attendance","LateSchoolFinishPercentOfQuarter"),
            new LayoutKey("Arrivals","ModerateFinishPercentOfQuarter","Dynamic Gameday Attendance","ModerateFinishPercentOfQuarter"),
            new LayoutKey("Arrivals","HalftimeSeatBreakStrength","Dynamic Gameday Attendance","HalftimeSeatBreakStrength"),
            new LayoutKey("Arrivals","BigGameBonus","Dynamic Gameday Attendance","BigGameBonus"),
            new LayoutKey("Arrivals","BadWeatherPenalty","Dynamic Gameday Attendance","BadWeatherPenalty"),
            new LayoutKey("Arrivals","PlayoffStartPercent","Dynamic Gameday Attendance","PlayoffStartPercent"),
            new LayoutKey("Arrivals","ChampionshipStartPercent","Dynamic Gameday Attendance","ChampionshipStartPercent"),
            new LayoutKey("Arrivals","FcsOpponentPenalty","Dynamic Gameday Attendance","FcsOpponentPenalty"),
            new LayoutKey("Audio","Enabled","Dynamic Gameday Soundscape","Enabled"),
            new LayoutKey("Audio","ReductionStartPercent","Dynamic Gameday Soundscape","ReductionStartPercent"),
            new LayoutKey("Audio","NativeQuietingStrengthPercent","Dynamic Gameday Soundscape","NativeQuietingStrengthPercent"),
            new LayoutKey("Audio","CrowdVolumeReductionPercent","Dynamic Gameday Soundscape","CrowdVolumeReductionPercent"),
            new LayoutKey("Audio","MaximumReductionPercent","Dynamic Gameday Soundscape","MaximumReductionPercent"),
            new LayoutKey("Atmosphere","Enabled","Dynamic Gameday Soundscape","LayeredAtmosphereEnabled"),
            new LayoutKey("Atmosphere","OpeningAtmosphere","Dynamic Gameday Soundscape","OpeningAtmosphere"),
            new LayoutKey("Atmosphere","WinningEruptions","Dynamic Gameday Soundscape","WinningEruptions"),
            new LayoutKey("Atmosphere","OvertimeAtmosphere","Dynamic Gameday Soundscape","OvertimeAtmosphere"),
            new LayoutKey("Atmosphere","PlayCallAtmosphere","Dynamic Gameday Soundscape","PlayCallAtmosphere"),
            new LayoutKey("Atmosphere","UpsetWatchAtmosphere","Dynamic Gameday Soundscape","UpsetWatchAtmosphere"),
            new LayoutKey("Atmosphere","BigGameMinimumFullnessPercent","Dynamic Gameday Soundscape","BigGameMinimumFullnessPercent"),
            new LayoutKey("Atmosphere","PackedHouseBuildup","Dynamic Gameday Soundscape","PackedHouseBuildup"),
            new LayoutKey("Atmosphere","DriveEscalation","Dynamic Gameday Soundscape","DriveEscalation"),
            new LayoutKey("Atmosphere","ChantMaximumBoostPercent","Dynamic Gameday Soundscape","ChantMaximumBoostPercent"),
            new LayoutKey("Atmosphere","MaximumBoostPercent","Dynamic Gameday Soundscape","MaximumBoostPercent"),
            new LayoutKey("Atmosphere","HighLeverageBoostPercent","Dynamic Gameday Soundscape","HighLeverageBoostPercent"),
            new LayoutKey("Atmosphere","EruptionBoostPercent","Dynamic Gameday Soundscape","EruptionBoostPercent"),
            new LayoutKey("Atmosphere","NativeShakeMinimumFullnessPercent","Dynamic Gameday Broadcast","NativeShakeMinimumFullnessPercent"),
            new LayoutKey("Atmosphere","CameraShake","Dynamic Gameday Broadcast","CameraShake"),
            new LayoutKey("Atmosphere","OrdinaryShakeStrengthPercent","Dynamic Gameday Broadcast","OrdinaryShakeStrengthPercent"),
            new LayoutKey("Atmosphere","CriticalShakeStrengthPercent","Dynamic Gameday Broadcast","CriticalShakeStrengthPercent"),
            new LayoutKey("Atmosphere","TouchdownShakeStrengthPercent","Dynamic Gameday Broadcast","TouchdownShakeStrengthPercent"),
            new LayoutKey("Atmosphere","EruptionShakeStrengthPercent","Dynamic Gameday Broadcast","EruptionShakeStrengthPercent"),
            new LayoutKey("Atmosphere","TouchdownShakeImpactPercent","Dynamic Gameday Broadcast","TouchdownShakeImpactPercent"),
            new LayoutKey("Atmosphere","EruptionShakeImpactPercent","Dynamic Gameday Broadcast","EruptionShakeImpactPercent"),
            new LayoutKey("Atmosphere","TouchdownShakeDurationSeconds","Dynamic Gameday Broadcast","TouchdownShakeDurationSeconds"),
            new LayoutKey("Atmosphere","EruptionShakeDurationSeconds","Dynamic Gameday Broadcast","EruptionShakeDurationSeconds"),
            new LayoutKey("Replays","Enabled","Dynamic Gameday Broadcast","ExtraReplaysEnabled"),
            new LayoutKey("Replays","InsetAfterRoutinePlays","Dynamic Gameday Broadcast","InsetAfterRoutinePlays"),
            new LayoutKey("Replays","FullScreenBigPlays","Dynamic Gameday Broadcast","FullScreenBigPlays"),
            new LayoutKey("Replays","FullScreenMinimumYards","Dynamic Gameday Broadcast","FullScreenMinimumYards"),
            new LayoutKey("Replays","DefensiveHighlights","Dynamic Gameday Broadcast","DefensiveHighlights"),
        };
        public static void Category(string section,string key,out string category,out string name){
            foreach(var item in Layout)if(String.Equals(section,item.OldSection,StringComparison.OrdinalIgnoreCase)&&String.Equals(key,item.OldKey,StringComparison.OrdinalIgnoreCase)){category=item.Section;name=item.Key;return;}
            category=section;name=key;
        }
        sealed class LayoutEntry {public string Section,Key,Raw;public List<string> Comments;public int Rank;}
        static int Order(LayoutEntry e,bool current){
            if(String.Equals(e.Section,"General",StringComparison.OrdinalIgnoreCase)){var keys=new[]{"ConfigVersion","AudioBalanceVersion","AttendanceBalanceVersion","ReplayBalanceVersion"};for(int i=0;i<keys.Length;i++)if(e.Key==keys[i])return i;}
            if(String.Equals(e.Section,"Logging",StringComparison.OrdinalIgnoreCase))return e.Key=="MaxFileMB"?0:1;
            for(int i=0;i<Layout.Length;i++){var k=Layout[i];if(String.Equals(e.Section,current?k.Section:k.OldSection,StringComparison.OrdinalIgnoreCase)&&String.Equals(e.Key,current?k.Key:k.OldKey,StringComparison.OrdinalIgnoreCase))return i*10;
                if(current&&e.Section==k.Section){if(k.OldKey=="CriticalShakeStrengthPercent"&&e.Key=="DuringPlayShakeStrengthPercent")return i*10+5;if(k.OldKey=="LateSchoolStartPercent"&&e.Key=="BigGameStartPercent")return i*10+5;if(k.OldSection=="Atmosphere"&&k.OldKey=="Enabled"){if(e.Key=="DuringPlayReactionsEnabled")return i*10+3;if(e.Key=="LongCompletionMinimumYards")return i*10+4;if(e.Key=="LongGainMinimumYards")return i*10+5;}}
            }
            return 10000;
        }
        static string Organize(string text,bool current){
            var sections=new Dictionary<string,List<LayoutEntry>>(StringComparer.OrdinalIgnoreCase);
            var entries=new Dictionary<string,LayoutEntry>(StringComparer.OrdinalIgnoreCase);
            var notes=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null){section=next;continue;}
                string key=Key(raw);if(key==null){notes.Add(raw);continue;}
                LayoutKey map=null;bool fromCurrent=false;
                foreach(var k in Layout){if(String.Equals(section,k.Section,StringComparison.OrdinalIgnoreCase)&&String.Equals(key,k.Key,StringComparison.OrdinalIgnoreCase)){map=k;fromCurrent=true;break;}}
                if(map==null)foreach(var k in Layout){if(String.Equals(section,k.OldSection,StringComparison.OrdinalIgnoreCase)&&String.Equals(key,k.OldKey,StringComparison.OrdinalIgnoreCase)){map=k;break;}}
                string target=map==null?section:current?map.Section:map.OldSection;
                string targetKey=map==null?key:current?map.Key:map.OldKey;
                string id=target+"\0"+targetKey;int rank=fromCurrent?2:1;LayoutEntry old;
                while(notes.Count>0&&String.IsNullOrWhiteSpace(notes[0]))notes.RemoveAt(0);while(notes.Count>0&&String.IsNullOrWhiteSpace(notes[notes.Count-1]))notes.RemoveAt(notes.Count-1);
                var item=new LayoutEntry{Section=target,Key=targetKey,Raw=targetKey+raw.Substring(raw.IndexOf('=')),Comments=new List<string>(notes),Rank=rank};notes.Clear();
                if(entries.TryGetValue(id,out old)){
                    if(rank>old.Rank){old.Raw=item.Raw;old.Comments.AddRange(item.Comments);old.Rank=rank;}
                    else old.Comments.AddRange(item.Comments);
                    continue;
                }
                if(!sections.ContainsKey(target))sections[target]=new List<LayoutEntry>();sections[target].Add(item);entries[id]=item;
            }
            string[] order=current?new[]{"General","Dynamic Gameday Attendance","Dynamic Gameday Soundscape","Dynamic Gameday Broadcast","Logging"}:new[]{"General","Attendance","Retention","Arrivals","Audio","Atmosphere","Replays","Logging"};
            var names=new List<string>();foreach(string n in order)if(sections.ContainsKey(n))names.Add(n);
            foreach(string n in sections.Keys)if(!names.Exists(x=>String.Equals(x,n,StringComparison.OrdinalIgnoreCase)))names.Add(n);
            var output=new List<string>();foreach(string n in names){output.Add("");output.Add("["+n+"]");sections[n].Sort((a,b)=>{int rank=Order(a,current).CompareTo(Order(b,current));return rank!=0?rank:StringComparer.OrdinalIgnoreCase.Compare(a.Key,b.Key);});foreach(var e in sections[n]){if(e.Comments.Count>0)output.Add("");output.AddRange(e.Comments);output.Add(e.Raw);}}
            while(notes.Count>0&&String.IsNullOrWhiteSpace(notes[0]))notes.RemoveAt(0);while(notes.Count>0&&String.IsNullOrWhiteSpace(notes[notes.Count-1]))notes.RemoveAt(notes.Count-1);
            output.AddRange(notes);return String.Join(Environment.NewLine,output).Trim()+Environment.NewLine;
        }
        static string Section(string line){string s=line.Trim();return s.StartsWith("[")&&s.EndsWith("]")?s.Substring(1,s.Length-2).Trim():null;}
        static string Key(string line){string s=line.Trim();int eq=s.IndexOf('=');return s.StartsWith(";")||s.StartsWith("#")||eq<1?null:s.Substring(0,eq).Trim();}
        internal static int Number(string text,string wantedSection,string wantedKey,int fallback){
            string section="";foreach(string line in text.Replace("\r","").Split('\n')){
                string next=Section(line);if(next!=null){section=next;continue;}
                if(String.Equals(section,wantedSection,StringComparison.OrdinalIgnoreCase)&&String.Equals(Key(line),wantedKey,StringComparison.OrdinalIgnoreCase)){
                    int value;string raw=line.Substring(line.IndexOf('=')+1).Split(';','#')[0].Trim();
                    return Int32.TryParse(raw,out value)?value:fallback;
                }
            }return fallback;
        }
        static double RealNumber(string text,string wantedSection,string wantedKey,double fallback){
            string section="";foreach(string line in text.Replace("\r","").Split('\n')){
                string next=Section(line);if(next!=null){section=next;continue;}
                if(String.Equals(section,wantedSection,StringComparison.OrdinalIgnoreCase)&&String.Equals(Key(line),wantedKey,StringComparison.OrdinalIgnoreCase)){
                    double value;string raw=line.Substring(line.IndexOf('=')+1).Split(';','#')[0].Trim();
                    return Double.TryParse(raw,NumberStyles.Float,CultureInfo.InvariantCulture,out value)&&!Double.IsNaN(value)&&!Double.IsInfinity(value)?value:fallback;
                }
            }return fallback;
        }
        static string RetuneOldDefaults(string existing,string defaults){
            int version=Number(defaults,"General","AudioBalanceVersion",0);
            if(version<2||Number(existing,"General","AudioBalanceVersion",0)>=2||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=2";continue;}
                if(!String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase))continue;
                int old=String.Equals(key,"NormalBoostDb",StringComparison.OrdinalIgnoreCase)?3:
                    String.Equals(key,"PeakBoostDb",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"MaximumBoostDb",StringComparison.OrdinalIgnoreCase)?6:-1;
                int value;if(old<0||!Int32.TryParse(lines[i].Substring(lines[i].IndexOf('=')+1).Split(';','#')[0].Trim(),out value)||value!=old)continue;
                lines[i]=key+"="+Number(defaults,"Energy",key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneGamedayDefaults(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<3||Number(existing,"General","AudioBalanceVersion",0)>=3||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=3";continue;}
                int old=String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"MaximumBoostDb",StringComparison.OrdinalIgnoreCase)?12:
                    String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"StartPercent",StringComparison.OrdinalIgnoreCase)?85:-1;
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+Number(defaults,section,key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneHalftimeDefault(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<4||Number(existing,"General","AudioBalanceVersion",0)>=4||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]=key+"=4";
                if(String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"HalftimeSeatBreakPercent",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==3)
                    lines[i]=key+"="+Number(defaults,section,key,10);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneReleaseDefaults(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<5||Number(existing,"General","AudioBalanceVersion",0)>=5||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=5";continue;}
                // Keep inert legacy keys during an update: an already-running
                // older DLL must not fall back to its former loud defaults.
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"CommentaryFollowPercent",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=0";continue;}
                if(String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"BoostAmplitudePercent",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=100";continue;}
                int old=String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"MaximumBoostDb",StringComparison.OrdinalIgnoreCase)?15:
                    String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"StartPercent",StringComparison.OrdinalIgnoreCase)?70:
                    String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"LateSchoolStartPercent",StringComparison.OrdinalIgnoreCase)?60:-1;
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+Number(defaults,section,key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneEnergyDefaults(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<6||Number(existing,"General","AudioBalanceVersion",0)>=6||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]=key+"=6";
                if(String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AllCrowdBaselineDb",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==2)
                    lines[i]=key+"="+Number(defaults,section,key,3);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneAtmosphereDefaults(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<7||Number(existing,"General","AudioBalanceVersion",0)>=7||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=7";continue;}
                bool energy=String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase);
                if(energy&&String.Equals(key,"AllCrowdBaselineDb",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=0";continue;}
                if(energy&&String.Equals(key,"MaximumBoostDb",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,0)>8){lines[i]=key+"=8";continue;}
                int old=String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"Strength",StringComparison.OrdinalIgnoreCase)?35:-1;
                if(energy){
                    if(String.Equals(key,"NormalBoostDb",StringComparison.OrdinalIgnoreCase))old=5;
                    if(String.Equals(key,"BigGameBoostDb",StringComparison.OrdinalIgnoreCase))old=6;
                    if(String.Equals(key,"RankedRivalryBoostDb",StringComparison.OrdinalIgnoreCase))old=8;
                    if(String.Equals(key,"PlayoffBoostDb",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"OvertimeBoostDb",StringComparison.OrdinalIgnoreCase))old=9;
                }
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+Number(defaults,section,key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneOpeningDefaults(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<8||Number(existing,"General","AudioBalanceVersion",0)>=8||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=8";continue;}
                if(!String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase))continue;
                if(String.Equals(key,"MaximumBoostDb",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,0)>6){lines[i]=key+"=6";continue;}
                if(String.Equals(key,"BigGameAmbienceExtraDb",StringComparison.OrdinalIgnoreCase)){lines[i]=key+"=0";continue;}
                int old=String.Equals(key,"NormalBoostDb",StringComparison.OrdinalIgnoreCase)?4:
                    String.Equals(key,"PeakBoostDb",StringComparison.OrdinalIgnoreCase)?8:
                    String.Equals(key,"BigGameBoostDb",StringComparison.OrdinalIgnoreCase)?3:
                    String.Equals(key,"RankedRivalryBoostDb",StringComparison.OrdinalIgnoreCase)?4:
                    String.Equals(key,"PlayoffBoostDb",StringComparison.OrdinalIgnoreCase)?5:
                    String.Equals(key,"ChampionshipBoostDb",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"OvertimeBoostDb",StringComparison.OrdinalIgnoreCase)?6:-1;
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+Number(defaults,section,key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneCoreAudio(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<9||Number(existing,"General","AudioBalanceVersion",0)>=9||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            int enabled=Number(existing,"Energy","Enabled",1);
            int opening=Number(existing,"Energy","SustainedRoars",1)==0?0:Number(existing,"Energy","OpeningAtmosphere",1);
            int eruptions=Number(existing,"Energy","PlaySurges",1);
            int normal=Math.Max(0,Math.Min(4,Number(existing,"Energy","MaximumBoostDb",4)));
            int decisive=Math.Max(0,Math.Min(6,Number(existing,"Energy","DecisiveMaximumBoostDb",6)));
            var lines=new List<string>();string section="";bool core=false;
            foreach(string line in existing.Replace("\r","").Split('\n')){
                string next=Section(line);if(next!=null){section=next;if(String.Equals(next,"Atmosphere",StringComparison.OrdinalIgnoreCase))core=true;}
                if(String.Equals(section,"Energy",StringComparison.OrdinalIgnoreCase))continue;
                string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines.Add("AudioBalanceVersion=9");continue;}
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)&&
                   (String.Equals(key,"CloseGameBoostPercent",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"MatchupBoostPercent",StringComparison.OrdinalIgnoreCase)||
                    String.Equals(key,"CommentaryFollowPercent",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"ChantHalfCrowdAdjustment",StringComparison.OrdinalIgnoreCase)))continue;
                lines.Add(line);
            }
            if(!core){lines.Add("[Atmosphere]");lines.Add("Enabled="+enabled);lines.Add("OpeningRoar="+opening);lines.Add("WinningEruptions="+eruptions);lines.Add("MaximumBoostDb="+normal);lines.Add("EruptionBoostDb="+decisive);}
            // Disable a previously loaded runtime while it waits for the required restart.
            // Only upgraded files need this compatibility key; fresh settings omit it.
            lines.Add("[Energy]");lines.Add("; Retired audio system. Kept off for an older DLL until the game restarts.");lines.Add("Enabled=0");
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneReductionStrength(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<10||Number(existing,"General","AudioBalanceVersion",0)>=10||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]="AudioBalanceVersion=10";
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"Strength",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==20)lines[i]="Strength=35";
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneGentleAudio(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<11||Number(existing,"General","AudioBalanceVersion",0)>=11||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]="AudioBalanceVersion=11";
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"Strength",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==35)lines[i]="Strength=15";
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetunePercentageAudio(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<12||Number(existing,"General","AudioBalanceVersion",0)>=12||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            bool explicitPercent=Number(existing,"Audio","MaximumReductionPercent",-1)>=0;
            int legacy=Math.Max(0,Math.Min(100,Number(existing,"Audio","Strength",15)));
            int percent=Math.Max(0,Math.Min(100,(int)Math.Round(legacy*20.0/15.0,MidpointRounding.AwayFromZero)));
            foreach(string line in existing.Replace("\r","").Split('\n')){
                string next=Section(line);if(next!=null){section=next;lines.Add(line);continue;}
                string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines.Add("AudioBalanceVersion=12");continue;}
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)){
                    if(String.Equals(key,"Strength",StringComparison.OrdinalIgnoreCase)){if(!explicitPercent){lines.Add("MaximumReductionPercent="+percent);explicitPercent=true;}continue;}
                    string trimmed=line.TrimStart();
                    if(trimmed.StartsWith("; Attendance-based quieting",StringComparison.OrdinalIgnoreCase))continue;
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetunePercentageMix(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<13||Number(existing,"General","AudioBalanceVersion",0)>=13||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            bool normal=RealNumber(existing,"Atmosphere","MaximumBoostPercent",-1)>=0;
            bool eruption=RealNumber(existing,"Atmosphere","EruptionBoostPercent",-1)>=0;
            bool chant=RealNumber(existing,"Audio","ChantReductionPercent",-1)>=0;
            var lines=new List<string>();string section="";
            foreach(string line in existing.Replace("\r","").Split('\n')){
                string next=Section(line);if(next!=null){section=next;lines.Add(line);continue;}
                string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase)){lines.Add("AudioBalanceVersion=13");continue;}
                if(String.Equals(section,"Atmosphere",StringComparison.OrdinalIgnoreCase)){
                    if(String.Equals(key,"MaximumBoostDb",StringComparison.OrdinalIgnoreCase)){
                        if(!normal){double db=Math.Max(0,Math.Min(4,RealNumber(existing,section,key,4)));lines.Add("MaximumBoostPercent="+(100*(Math.Pow(10,db/20)-1)).ToString("0.##",CultureInfo.InvariantCulture));normal=true;}continue;
                    }
                    if(String.Equals(key,"EruptionBoostDb",StringComparison.OrdinalIgnoreCase)){
                        if(!eruption){double db=Math.Max(0,Math.Min(6,RealNumber(existing,section,key,6)));lines.Add("EruptionBoostPercent="+(100*(Math.Pow(10,db/20)-1)).ToString("0.##",CultureInfo.InvariantCulture));eruption=true;}continue;
                    }
                }
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"ChantReductionDb",StringComparison.OrdinalIgnoreCase)){
                    if(!chant){double db=Math.Max(0,Math.Min(24,RealNumber(existing,section,key,6)));double cut=db==6?50:100*(1-Math.Pow(10,-db/20));lines.Add("ChantReductionPercent="+cut.ToString("0.##",CultureInfo.InvariantCulture));chant=true;}continue;
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneAttendanceDefaults(string existing,string defaults){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<1||Number(existing,"General","AttendanceBalanceVersion",0)>=1||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AttendanceBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]=key+"=1";
                if(!String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase))continue;
                int old=String.Equals(key,"StartPercent",StringComparison.OrdinalIgnoreCase)?60:
                    String.Equals(key,"FinishPercentOfQuarter",StringComparison.OrdinalIgnoreCase)?50:
                    String.Equals(key,"LateSchoolFinishPercentOfQuarter",StringComparison.OrdinalIgnoreCase)?85:
                    String.Equals(key,"BigGameBonus",StringComparison.OrdinalIgnoreCase)?10:-1;
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+Number(defaults,section,key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneArrivalBaselines(string existing,string defaults){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<2||Number(existing,"General","AttendanceBalanceVersion",0)>=2||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AttendanceBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]=key+"=2";
                if(!String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase))continue;
                int old=String.Equals(key,"StartPercent",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"PlayoffStartPercent",StringComparison.OrdinalIgnoreCase)?70:
                    String.Equals(key,"LateSchoolStartPercent",StringComparison.OrdinalIgnoreCase)?50:
                    String.Equals(key,"ModerateStartPercent",StringComparison.OrdinalIgnoreCase)?60:
                    String.Equals(key,"ChampionshipStartPercent",StringComparison.OrdinalIgnoreCase)?80:-1;
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+Number(defaults,section,key,old);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneQuarterCap(string existing,string defaults){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<3||Number(existing,"General","AttendanceBalanceVersion",0)>=3||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AttendanceBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]=key+"=3";
                if(String.Equals(section,"Retention",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"MaximumDeparturePerQuarter",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==30)
                    lines[i]=key+"="+Number(defaults,section,key,45);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneAttendanceBalance4(string existing,string defaults){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<4||Number(existing,"General","AttendanceBalanceVersion",0)>=4||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw;string next=Section(line);if(next!=null)section=next;
                string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AttendanceBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=4";
                if(String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase)){
                    int old=String.Equals(key,"StartPercent",StringComparison.OrdinalIgnoreCase)?65:
                        String.Equals(key,"ModerateStartPercent",StringComparison.OrdinalIgnoreCase)?55:
                        String.Equals(key,"LateSchoolStartPercent",StringComparison.OrdinalIgnoreCase)?45:-1;
                    if(old>=0&&Number(existing,section,key,-1)==old)line=key+"="+Number(defaults,section,key,old);
                    if(String.Equals(key,"HalftimeSeatBreakPercent",StringComparison.OrdinalIgnoreCase)){
                        if(Number(existing,section,"HalftimeSeatBreakStrength",-1)>=0)continue;
                        line="HalftimeSeatBreakStrength="+Math.Max(0,Math.Min(200,Number(existing,section,key,10)*10));
                    }
                }
                if(String.Equals(section,"Retention",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"ComebackOriginalDeficit",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==28)line=key+"=21";
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneReplayDefaults(string existing,string defaults){
            if(Number(defaults,"General","ReplayBalanceVersion",0)<1||Number(existing,"General","ReplayBalanceVersion",0)>=1||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"ReplayBalanceVersion",StringComparison.OrdinalIgnoreCase))lines[i]=key+"=1";
                if(String.Equals(section,"Replays",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"FullScreenMinimumYards",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,-1)==20)
                    lines[i]=key+"="+Number(defaults,section,key,15);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneAttendanceAudio14(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<14||Number(existing,"General","AudioBalanceVersion",0)>=14||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw;string next=Section(line);if(next!=null)section=next;
                string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=14";
                if(String.Equals(section,"Atmosphere",StringComparison.OrdinalIgnoreCase)){
                    if(String.Equals(key,"OpeningRoar",StringComparison.OrdinalIgnoreCase)){if(Number(existing,section,"OpeningAtmosphere",-1)>=0)continue;line="OpeningAtmosphere="+Number(existing,section,key,1);}
                    if(String.Equals(key,"OvertimeRoar",StringComparison.OrdinalIgnoreCase)){if(Number(existing,section,"OvertimeAtmosphere",-1)>=0)continue;line="OvertimeAtmosphere="+Number(existing,section,key,1);}
                    if(String.Equals(key,"PlayCallBoostPercent",StringComparison.OrdinalIgnoreCase))continue;
                    if(String.Equals(key,"MaximumBoostPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+Math.Max(0,Math.Min(50,RealNumber(existing,section,key,50))).ToString("0.###",CultureInfo.InvariantCulture);
                    if(String.Equals(key,"EruptionBoostPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+Math.Max(0,Math.Min(70,RealNumber(existing,section,key,70))).ToString("0.###",CultureInfo.InvariantCulture);
                }
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)){
                    if(String.Equals(key,"ChantStrength",StringComparison.OrdinalIgnoreCase)||String.Equals(key,"ChantReductionPercent",StringComparison.OrdinalIgnoreCase))continue;
                    if(String.Equals(key,"ReductionStartPercent",StringComparison.OrdinalIgnoreCase))line=key+"=40";
                    if(String.Equals(key,"MaximumReductionPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+Math.Max(0,Math.Min(20,Number(existing,section,key,20)));
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneVolumeTiers15(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<15||Number(existing,"General","AudioBalanceVersion",0)>=15||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            bool high=RealNumber(existing,"Atmosphere","HighLeverageBoostPercent",-1)>=0;
            double normal=Math.Max(0,Math.Min(25,RealNumber(existing,"Atmosphere","MaximumBoostPercent",25)));
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw,next=Section(line);if(next!=null)section=next;string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=15";
                if(String.Equals(section,"Atmosphere",StringComparison.OrdinalIgnoreCase)){
                    if(next!=null&&!high){lines.Add(line);lines.Add("HighLeverageBoostPercent="+Math.Min(35,normal*1.4).ToString("0.###",CultureInfo.InvariantCulture));continue;}
                    if(String.Equals(key,"MaximumBoostPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+normal.ToString("0.###",CultureInfo.InvariantCulture);
                    if(String.Equals(key,"HighLeverageBoostPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+Math.Max(0,Math.Min(35,RealNumber(existing,section,key,35))).ToString("0.###",CultureInfo.InvariantCulture);
                    if(String.Equals(key,"EruptionBoostPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+Math.Max(0,Math.Min(50,RealNumber(existing,section,key,50))).ToString("0.###",CultureInfo.InvariantCulture);
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneCapacityArrivals5(string existing,string defaults){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<5||Number(existing,"General","AttendanceBalanceVersion",0)>=5||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw,next=Section(line);if(next!=null)section=next;string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AttendanceBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=5";
                if(String.Equals(section,"Arrivals",StringComparison.OrdinalIgnoreCase)){
                    int old=String.Equals(key,"StartPercent",StringComparison.OrdinalIgnoreCase)?70:
                        String.Equals(key,"ModerateStartPercent",StringComparison.OrdinalIgnoreCase)?60:
                        String.Equals(key,"LateSchoolStartPercent",StringComparison.OrdinalIgnoreCase)?50:-1;
                    if(old>=0){int value=Number(existing,section,key,old);if(value==old)value=Number(defaults,section,key,old);line=key+"="+Math.Max(30,Math.Min(80,value));}
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneRenderedAudio17(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<17||Number(existing,"General","AudioBalanceVersion",0)>=17||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw,next=Section(line);if(next!=null)section=next;string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=17";
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneRenderedAudio18(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<18||Number(existing,"General","AudioBalanceVersion",0)>=18||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw,next=Section(line);if(next!=null)section=next;string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=18";
                if(String.Equals(section,"Audio",StringComparison.OrdinalIgnoreCase)){
                    if(String.Equals(key,"ReductionStartPercent",StringComparison.OrdinalIgnoreCase))line=key+"="+Math.Max(1,Math.Min(25,Number(existing,section,key,25)));
                    if(String.Equals(key,"MaximumReductionPercent",StringComparison.OrdinalIgnoreCase)&&Number(existing,section,key,100)==20)line=key+"=100";
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneLineupAudio19(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<19||Number(existing,"General","AudioBalanceVersion",0)>=19||
               Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw,next=Section(line);if(next!=null)section=next;string key=Key(line);
                if(String.Equals(section,"General",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"AudioBalanceVersion",StringComparison.OrdinalIgnoreCase))line=key+"=19";
                if(String.Equals(section,"Atmosphere",StringComparison.OrdinalIgnoreCase)&&String.Equals(key,"ChantMaximumBoostPercent",StringComparison.OrdinalIgnoreCase)){
                    double old=RealNumber(existing,section,key,15);if(old==10)line=key+"=15";
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1380(string existing,string defaults,string original){
            bool arrivals=Number(original,"General","AttendanceBalanceVersion",0)<6&&Number(defaults,"General","AttendanceBalanceVersion",0)>=6;
            bool audio=Number(original,"General","AudioBalanceVersion",0)<21&&Number(defaults,"General","AudioBalanceVersion",0)>=21;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;string key=Key(raw),line=raw;
                if(arrivals&&section=="Arrivals"){
                    if(key=="ModerateStartPercent"||key=="ModerateFinishPercentOfQuarter"||key=="PlayoffStartPercent"||key=="ChampionshipStartPercent"){while(lines.Count>0&&(String.IsNullOrWhiteSpace(lines[lines.Count-1])||lines[lines.Count-1].TrimStart().StartsWith(";")||lines[lines.Count-1].TrimStart().StartsWith("#")))lines.RemoveAt(lines.Count-1);continue;}
                    if(key=="StartPercent"&&Number(existing,section,key,0)==60)line=key+"=55";
                    if(key=="BigGameBonus"&&Number(existing,section,key,0)==20)line=key+"=10";
                }
                if(audio&&section=="Atmosphere"){
                    if(key=="EruptionShakeStrengthPercent"&&RealNumber(existing,section,key,0)==30)line=key+"=22.5";
                    if(key=="EruptionShakeImpactPercent"&&Number(existing,section,key,0)==40)line=key+"=30";
                }
                if(audio&&section=="Dynamic Gameday Soundscape"&&key=="LongGainMinimumYards"&&Number(existing,section,key,0)==30)line=key+"=25";
                if(section=="General"){
                    if(arrivals&&key=="AttendanceBalanceVersion")line=key+"=6";
                    if(audio&&key=="AudioBalanceVersion")line=key+"=21";
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines)
                .Replace("; Standard schools' starting percentage before matchup/weather adjustments: 30-80.","; Normal schools before a combined matchup/weather adjustment of -10 to +10 points.")
                .Replace("; 75 = three quarters of the way through. Matchup and weather adjust the timing.","; 75 = three quarters of the way through Q1; weather/matchup do not change the finish point.")
                .Replace("; How much an attractive matchup can improve starting crowds and arrival speed: 0-20.","; Rivalry starting-crowd bonus, capped at 10 points. Rivalry alone is not a big game.")
                .Replace("; Starting-crowd penalty for rain/snow; it also slows arrivals. Range: 0-30.","; Rain/snow starting-crowd penalty, capped at 10 points; fill timing stays unchanged.")
                .Replace("; The regular-season FCS rule also includes a separate small built-in penalty.","; The FCS penalty is included within the combined -10/+10 starting adjustment.")
                .Replace("; Minimum seating fullness for rivalry, both-Top-25 and postseason buildup: 35-85.","; Minimum fullness after the opening for both-Top-25/CFP/championship buildup.")
                .Replace("; 60 lets big games start sounding excited while the first-quarter crowd fills in.","; Big games qualify immediately during the opening once mapped seating is ready; strength follows fullness.")
                .Replace("; Default 16: 40% opening hit, 30% hold through about 8 seconds, then an 8-second fade.","; Default 16: 30% opening hit, 22.5% hold through about 8 seconds, then an 8-second fade.");
        }
        static string RefreshHelp(string existing,string defaults){
            // Upgrade help for known keys once. Values, unknown keys/sections
            // and comments attached to unknown settings remain as supplied.
            var help=new Dictionary<string,List<string>>(StringComparer.OrdinalIgnoreCase);
            var pending=new List<string>();string section="";
            foreach(string raw in defaults.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null){section=next;pending.Clear();continue;}
                string key=Key(raw);
                if(key!=null){help[section+"\0"+key]=new List<string>(pending);pending.Clear();}
                else if(raw.TrimStart().StartsWith(";")||raw.TrimStart().StartsWith("#"))pending.Add(raw);
                else pending.Clear();
            }
            var stock=new HashSet<string>(StringComparer.Ordinal);
            // Retired stock help from 1.82; retain custom user notes.
            foreach(string note in new[]{"; Advanced fallback marquee target; its bonus is limited by BigGameBonus and the 70% regular cap.","; Guide: Default 0; range 0-5 points of planned crowd; whole numbers only.","; Guide: Default 40; range 40-50 % of planned crowd; whole numbers only.","; Guide: Default 52; range 40-70 % of planned crowd; whole numbers only.","; Guide: Default 7; range 0-7 points of planned crowd; whole numbers only.","; Optional fallback FCS delay; never takes a dry kickoff below 40%.","; Ordinary kickoff is 40-55% of expected peak; severe-late baseline 40%.","; Regular cap 70%, weather-only floor 35%; postseason 70-80%, championships 80-90%.","; Revised fresh launches calculate 40-50% of peak by demand; night adds up to 5.","; Revised turnout already accounts for FCS opponent appeal, without repeating the penalty."})stock.Add(note);
            // Retired stock arrival help from 1.81; preserve other user comments.
            foreach(string note in new[]{"; For revised turnout, kickoff arrivals use90% normally /97% at night (78%/88%","; at known late-arrival schools). Legacy StartPercent settings remain the fallback.","; Normal schools start here before a combined matchup/weather adjustment of -10 to +10 points.","; Former moderate-late schools now use this group. Range 30-90; default 55.","; Guide: Default 55; range 30-90 % of the game's planned crowd; whole numbers only.","; Guide: Higher = more of the planned crowd seated at opening; lower = fewer. The game's original peak is unchanged.","; UCLA, Miami (FL), Georgia Tech and Maryland start here. Range 30-80; default 40.","; Guide: Default 40; range 30-80 % of the game's planned crowd; whole numbers only.","; Guide: Higher = more opening fans for UCLA, Miami, Georgia Tech and Maryland; lower = fewer. Peak attendance is unchanged.","; Normal schools finish their arrivals this far through Q1. Default 25 = one quarter of Q1.","; Guide: Default 25; range 10-100 % of Q1 elapsed; whole numbers only.","; Guide: Higher = normal arrivals finish later in Q1; lower = earlier. This is a share of quarter length, not seconds.","; Severe-late schools finish this far through Q1. Default 75 = three quarters of Q1.","; These timings scale with quarter length; matchup/weather change the start, not the finish time.","; Guide: Default 75; range 10-100 % of Q1 elapsed; whole numbers only.","; Guide: Higher = severe-late arrivals finish later in Q1; lower = earlier. Seat changes still wait for hidden opportunities.","; Both-Top-25 and postseason games share this baseline. Hard minimum 60; range 60-90.","; Rivalry can add up to 10 and bad weather subtracts up to 10, but never below 60.","; Guide: Default 60; range 60-90 % of the game's planned crowd; whole numbers only.","; Guide: Higher = a fuller big-game opening; lower = fewer opening fans, with a firm 60% minimum of planned attendance.","; Rivalry starting-crowd bonus, capped at 10 points. Rivalry alone no longer makes a big game.","; Guide: Default 10; range 0-10 points of original attendance; whole numbers only.","; Guide: Higher = a larger rivalry opening-attendance bonus; lower = smaller. This does not make a rivalry alone a big game.","; Rain/snow starting-crowd penalty, 0-10 points.","; Guide: Default 10; range 0-10 points of original attendance; whole numbers only.","; Guide: Higher = a larger rain/snow opening-attendance penalty; lower = smaller. The combined ordinary adjustment is limited to ten points.","; FCS starting-crowd penalty, 0-10. Included within the combined -10/+10 adjustment.","; Guide: Default 10; range 0-10 points of original attendance; whole numbers only.","; Guide: Higher = a larger FCS opening-attendance penalty; lower = smaller. It shares the combined ten-point adjustment limit."})stock.Add(note);
            foreach(var value in help.Values)foreach(string note in value)stock.Add(note.Trim());
            foreach(string note in new[]{"; 100 = linear to silence: 20% full is 20% quieter; 10% full is 60% quieter.","; Added live-play reactions require the layered atmosphere to qualify for that play.","; Camera motion after a qualifying long catch or interception: 0-30%.","; Default 25; allowed range 1-25. Only the emptier stadiums receive a cut.","; Default 9: 20% opening hit, 15% hold through about 5 seconds, then a 4-second fade.","; During the opening, big games qualify as soon as measured seating is ready and nonempty.","; Gentle movement during an eligible ordinary play-call buildup.","; On offense or defense, an eligible lineup gets an early cheer and 40-second buildup/fade.","; Only during packed-house/big-game buildup. Third down needs 7 or fewer yards.","; Opening eruption hit: 0-100. Default 40%, then settles to the 30% level above.","; Size of temporary halftime seat breaks: 100 = usual amount; 0 = disabled; max 200.","; Big games qualify immediately during the opening once mapped seating is ready; strength follows fullness.","; Minimum fullness after the opening for both-Top-25/CFP/championship buildup."})stock.Add(note);
            foreach(string note in new[]{"; Lets ordinary matchups get lighter play-call buildup when at least 85% full.","; Default 30. Immediately tackled catches receive a short impact and settling fade.","; Starts as the runner reaches this distance, not only after the whistle. Default 25.","; Maximum ordinary lineup motion. Each call chooses a stable random level from 0 to this value.","; Default 3 gives 0-3%, not the previous steady 10%. Third down needs 7 or fewer yards."})stock.Add(note);
            foreach(string note in new[]{"; Minimum seating fullness for both-Top-25/postseason buildup after the opening: 60-85.","; All added atmosphere now needs at least 60% measured fullness, including the opening.","; Sound strength still follows actual fullness; rivalry alone does not activate these layers.","; Independent close critical downs/red-zone chances still use 70%; OT/late drives 65%.","; Needs at least 65% seating fullness; ordinary departure calculations stop in OT.","; Added reactions and camera effects require layered atmosphere AND at least 60% measured fullness."})stock.Add(note);
            foreach(string note in new[]{"; Random motion ceiling on critical downs during packed-house/big-game buildup.","; Third down needs 7 or fewer yards."})stock.Add(note);
            foreach(string note in new[]{"; Gives eligible play-call buildup extra energy early in Q1, then settles it.","; This is part of the play-call system, not a separate pregame sound loop."})stock.Add(note);
            foreach(string note in new[]{"; Changes fade over about six seconds. Native reactions follow the same quieting as ambience.","; Pauses, replays and unsupported audio routes return to the game's own intensity."})stock.Add(note);
            stock.Add("; Suppress the game's own Stadium Pulse camera shake below this actual seating fullness.");
            stock.Add("; Default 50; range 0-100. 0 disables this filter. Fuller stadiums keep native shake.");
            stock.Add("; Camera presentation only: the Pulse meter and gameplay home-field advantage stay unchanged.");
            stock.Add("; Added DGA camera motion has its own switches/strengths below. Soundscape must be enabled.");
            stock.Add("; Guide: Default 50; range 0-100 % actual seating fullness; whole numbers only.");
            stock.Add("; Guide: Higher = native Stadium Pulse shake is suppressed in more sparse games; lower = less suppression. 0 disables this filter.");
            // Retired stock help from the previous Update 3 candidate.
            stock.Add("; Added reactions inherit the size-based fullness requirement of their qualifying play.");
            stock.Add("; Adds anticipation while a verified pass is in flight on an eligible layered play.");
            stock.Add("; Below the fullness threshold, venues with up to 10,000 mapped positions settle below 100 excitement.");
            stock.Add("; Changes fade over about six seconds. High native excitement receives an extra sparse-crowd peak trim.");
            stock.Add("; Keeps the existing atmosphere/fullness gates; no extra volume bonus, cheer or shake before the result.");
            stock.Add("; Landed hit sticks retain their impact. A fresh credited hurdle may settle after the whistle.");
            stock.Add("; This ceiling rises smoothly with stadium size. Sparse large bowls retain more crowd presence.");
            stock.Add("; Uses the same qualifying atmosphere/fullness, cooldowns and volume limits.");
            stock.Add("; hurdles, jukes, spins and broken tackles. Trucks use the native big-hit crowd sound.");
            stock.Add("; Changes fade over about six seconds. Strong native reactions keep their excitement.");
            stock.Add("; Required fullness scales from 99% at 6,060 positions to 60% at 61,888.");
            stock.Add("; Smaller venues use 99%; larger venues use 60%. Empty positions count toward size.");
            stock.Add("; Starts at no boost at 85% fullness, then rises in a straight line to this limit.");
            stock.Add("; Minimum seating fullness for both-Top-25/postseason buildup after the opening: 35-85.");
            stock.Add("; Most fans allowed to leave permanently in one ordinary quarter: 1-50 points");
            stock.Add("; of the original crowd. Postseason games also have their own lower limits.");
            foreach(string note in new[]{"; In the last quarter of Q4, fans leave faster when a home comeback looks unlikely.","; With 12-minute quarters, this begins inside the final three minutes.","; trails by 28+ in the final 25% of Q4 (last 3 minutes with 12-minute quarters).","; Default 10 gives 5-10%, chosen once per call. Below 5 uses the configured value; 0 disables it."})stock.Add(note);
            var lines=new List<string>();section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                string key=Key(raw);List<string> comments;
                if(key!=null&&help.TryGetValue(section+"\0"+key,out comments)){
                    var customNotes=new List<string>();
                    while(lines.Count>0&&(lines[lines.Count-1].TrimStart().StartsWith(";")||lines[lines.Count-1].TrimStart().StartsWith("#"))){string note=lines[lines.Count-1];lines.RemoveAt(lines.Count-1);if(!stock.Contains(note.Trim()))customNotes.Insert(0,note);}
                    lines.AddRange(comments);lines.AddRange(customNotes);
                }
                lines.Add(raw);
            }
            return String.Join(Environment.NewLine,lines).TrimEnd()+Environment.NewLine;
        }
        static string Retune1390(string existing,string defaults,string original){
            bool audio=Number(original,"General","AudioBalanceVersion",0)<22&&Number(defaults,"General","AudioBalanceVersion",0)>=22;
            bool attendance=Number(original,"General","AttendanceBalanceVersion",0)<7&&Number(defaults,"General","AttendanceBalanceVersion",0)>=7;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;string key=Key(raw),line=raw;
                if(audio&&(section=="Audio"||section=="Dynamic Gameday Soundscape")&&key=="ReductionStartPercent"&&Number(existing,section,key,0)==25)line=key+"=35";
                if(audio&&(section=="Atmosphere"||section=="Dynamic Gameday Soundscape")&&key=="BigGameMinimumFullnessPercent"&&Number(existing,section,key,60)<60)line=key+"=60";
                if(audio&&(section=="Atmosphere"||section=="Dynamic Gameday Broadcast")){
                    if(key=="BigGameMinimumFullnessPercent"&&Number(existing,section,key,60)<60)line=key+"=60";
                    if(key=="OrdinaryShakeStrengthPercent"&&RealNumber(existing,section,key,0)==5)line=key+"=3";
                    if(key=="CriticalShakeStrengthPercent"&&RealNumber(existing,section,key,0)==10)line=key+"=3";
                    if(key=="TouchdownShakeDurationSeconds"&&Number(existing,section,key,0)==9)line=key+"=11";
                }
                if(section=="General"){
                    if(audio&&key=="AudioBalanceVersion")line=key+"=22";
                    if(attendance&&key=="AttendanceBalanceVersion")line=key+"=7";
                }
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1400(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=23||Number(defaults,"General","AudioBalanceVersion",0)<23)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;string key=Key(raw),line=raw;
                if(section=="Dynamic Gameday Soundscape"){
                    if(key=="LongCompletionMinimumYards"&&Number(existing,section,key,0)==30)line=key+"=20";
                    if(key=="LongGainMinimumYards"&&Number(existing,section,key,0)==25)line=key+"=20";
                }
                if(section=="Dynamic Gameday Broadcast"||section=="Atmosphere"){
                    if(key=="OrdinaryShakeStrengthPercent"&&RealNumber(existing,section,key,0)==3)line=key+"=4";
                    if(key=="CriticalShakeStrengthPercent"&&RealNumber(existing,section,key,0)==3)line=key+"=10";
                }
                if(section=="General"&&key=="AudioBalanceVersion")line=key+"=23";
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1410(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=24||Number(defaults,"General","AudioBalanceVersion",0)<24)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;string key=Key(raw),line=raw;
                if((section=="Atmosphere"||section=="Dynamic Gameday Soundscape")&&key=="BigGameMinimumFullnessPercent"&&Number(existing,section,key,0)==60)line=key+"=55";
                if(section=="General"&&key=="AudioBalanceVersion")line=key+"=24";
                lines.Add(line);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1430(string existing,string defaults){
            if(Number(defaults,"General","AudioBalanceVersion",0)<26||Number(existing,"General","AudioBalanceVersion",0)>Number(defaults,"General","AudioBalanceVersion",0))return existing;
            var lines=new List<string>();string section="";
            int strength=Math.Max(0,Math.Min(100,Number(existing,"Audio","MaximumReductionPercent",100)));
            bool hasNative=Number(existing,"Audio","NativeQuietingStrengthPercent",-1)>=0;
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                if((section=="Audio"||section=="Dynamic Gameday Soundscape")&&Key(raw)=="MaximumReductionPercent")continue;
                lines.Add(raw);
            }
            string result=String.Join(Environment.NewLine,lines);
            if(!hasNative)result=Merge(result,"[Dynamic Gameday Soundscape]\nNativeQuietingStrengthPercent="+strength+"\n");
            return result;
        }
        static string Retune1431(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=27||Number(defaults,"General","AudioBalanceVersion",0)<27)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                bool threshold=(section=="Audio"||section=="Dynamic Gameday Soundscape")&&Key(raw)=="ReductionStartPercent";
                lines.Add(threshold&&Number(existing,section,"ReductionStartPercent",0)==35?"ReductionStartPercent=50":raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string AdvanceHelpVersion(string existing,string defaults,string original,int version){
            if(Number(original,"General","AudioBalanceVersion",0)>=version||Number(defaults,"General","AudioBalanceVersion",0)<version)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                lines.Add(section=="General"&&Key(raw)=="AudioBalanceVersion"?"AudioBalanceVersion="+version:raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1470(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=32||Number(defaults,"General","AudioBalanceVersion",0)<32)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                bool ordinary=(section=="Atmosphere"||section=="Dynamic Gameday Broadcast")&&Key(raw)=="OrdinaryShakeStrengthPercent";
                lines.Add(ordinary&&Number(existing,section,"OrdinaryShakeStrengthPercent",0)==4?"OrdinaryShakeStrengthPercent=5":raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1500(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=35||Number(defaults,"General","AudioBalanceVersion",0)<35)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                bool stock=section=="Dynamic Gameday Soundscape"&&Key(raw)=="LongGainMinimumYards"&&Number(existing,section,"LongGainMinimumYards",0)==20;
                lines.Add(stock?"LongGainMinimumYards=15":raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune1940(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=45||Number(defaults,"General","AudioBalanceVersion",0)<45)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                bool stock=section=="Dynamic Gameday Soundscape"&&Key(raw)=="LongGainMinimumYards"&&Number(existing,section,"LongGainMinimumYards",0)==15;
                lines.Add(stock?"LongGainMinimumYards=20":raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Retune2020(string existing,string defaults,string original){
            if(Number(original,"General","AudioBalanceVersion",0)>=55||Number(defaults,"General","AudioBalanceVersion",0)<55)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                bool stock=section=="Dynamic Gameday Soundscape"&&Key(raw)=="LongGainMinimumYards"&&Number(existing,section,"LongGainMinimumYards",0)==20;
                lines.Add(stock?"LongGainMinimumYards=15":raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string Merge(string existing,string defaults){
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            var pending=new List<string>();
            foreach(string raw in defaults.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null){section=next;pending.Clear();continue;}
                string key=Key(raw);if(key==null){
                    if(raw.TrimStart().StartsWith(";")||raw.TrimStart().StartsWith("#"))pending.Add(raw);else pending.Clear();
                    continue;
                }
                var addition=new List<string>(pending);addition.Add(raw);pending.Clear();
                string current="";int start=-1,end=lines.Count;bool found=false;
                for(int i=0;i<lines.Count;i++){
                    string name=Section(lines[i]);
                    if(name!=null){
                        if(start>=0&&!String.Equals(name,section,StringComparison.OrdinalIgnoreCase)){end=i;break;}
                        current=name;if(String.Equals(name,section,StringComparison.OrdinalIgnoreCase))start=i;
                    }
                    if(String.Equals(current,section,StringComparison.OrdinalIgnoreCase)&&String.Equals(Key(lines[i]),key,StringComparison.OrdinalIgnoreCase)){found=true;break;}
                }
                if(found)continue;
                if(start<0){lines.Add("");lines.Add("["+section+"]");lines.AddRange(addition);}
                else lines.InsertRange(end,addition);
            }
            return String.Join(Environment.NewLine,lines).TrimEnd()+Environment.NewLine;
        }
        static string Retune1558(string existing,string defaults,string original){
            bool upgrade=Number(original,"General","AudioBalanceVersion",0)<40&&Number(defaults,"General","AudioBalanceVersion",0)>=40;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string next=Section(raw);if(next!=null)section=next;
                string key=Key(raw);
                if(section=="Dynamic Gameday Soundscape"&&key=="ReactionDistanceExtraSeconds"){
                    while(lines.Count>0&&(lines[lines.Count-1].TrimStart().StartsWith(";")||lines[lines.Count-1].TrimStart().StartsWith("#")))lines.RemoveAt(lines.Count-1);
                    continue;
                }
                double fade;string value=key==null?"":raw.Substring(raw.IndexOf('=')+1).Split(';','#')[0].Trim();
                bool stock=upgrade&&section=="Dynamic Gameday Broadcast"&&key=="PlayReactionFadeSeconds"&&Double.TryParse(value,NumberStyles.Float,CultureInfo.InvariantCulture,out fade)&&Math.Abs(fade-4.5)<.0001;
                lines.Add(stock?"PlayReactionFadeSeconds=5":raw);
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneAttendance1820(string existing,string defaults,string original){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<8||Number(original,"General","AttendanceBalanceVersion",0)>=8)return existing;
            var lines=new List<string>(existing.Replace("\r","").Split('\n'));string section="";
            for(int i=0;i<lines.Count;i++){
                string next=Section(lines[i]);if(next!=null){section=next;continue;}
                string key=Key(lines[i]);if(key==null)continue;
                if(section=="General"&&key=="AttendanceBalanceVersion"){lines[i]=key+"=8";continue;}
                if(section!="Arrivals"&&section!="Dynamic Gameday Attendance")continue;
                int old=key=="StartPercent"?55:key=="FinishPercentOfQuarter"?25:key=="BigGameStartPercent"?60:
                    key=="BigGameBonus"||key=="BadWeatherPenalty"||key=="FcsOpponentPenalty"?10:-1;
                int value=key=="StartPercent"?45:key=="FinishPercentOfQuarter"?50:key=="BigGameStartPercent"?52:
                    key=="BigGameBonus"?7:key=="BadWeatherPenalty"?5:0;
                if(old>=0&&Number(existing,section,key,-1)==old)lines[i]=key+"="+value;
            }
            return String.Join(Environment.NewLine,lines);
        }
        static string RetuneAttendance1830(string existing,string defaults,string original){
            if(Number(defaults,"General","AttendanceBalanceVersion",0)<9||Number(original,"General","AttendanceBalanceVersion",0)>=9)return existing;
            var lines=new List<string>();string section="";
            foreach(string raw in existing.Replace("\r","").Split('\n')){
                string line=raw,next=Section(raw);if(next!=null)section=next;string key=Key(raw);
                if(section=="General"&&key=="AttendanceBalanceVersion")line=key+"=9";
                if(section=="Arrivals"||section=="Dynamic Gameday Attendance"){
                    if(key=="BigGameStartPercent"||key=="FcsOpponentPenalty")continue;
                    int old=key=="StartPercent"?45:key=="LateSchoolStartPercent"?40:key=="BigGameBonus"?7:-1;
                    int value=key=="StartPercent"?55:key=="LateSchoolStartPercent"?45:3;
                    if(old>=0&&Number(existing,section,key,-1)==old)line=key+"="+value;
                }lines.Add(line);
            }return String.Join(Environment.NewLine,lines);
        }
        private static string RetuneBusMixDefault(string text,string defaults,string existing) {
            if(Number(existing,"General","AudioBalanceVersion",0)>=47||Number(defaults,"General","AudioBalanceVersion",0)<47)return text;
            var lines=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')) {
                string header=Section(raw);if(header!=null)section=header;
                bool current=section=="Atmosphere"||section=="Dynamic Gameday Soundscape";
                lines.Add(current&&Key(raw)=="MaximumBoostPercent"&&Math.Abs(RealNumber(text,section,"MaximumBoostPercent",-1)-25)<0.00001?"MaximumBoostPercent=30":raw);
            }
            return String.Join("\n",lines.ToArray());
        }

        private static string RetuneShake2030(string text,string defaults,string existing) {
            if(Number(existing,"General","AudioBalanceVersion",0)>=48||Number(defaults,"General","AudioBalanceVersion",0)<48)return text;
            var stock=new Dictionary<string,double[]> {
                {"OrdinaryShakeStrengthPercent",new double[]{5,4}}, {"CriticalShakeStrengthPercent",new double[]{10,8}},
                {"DuringPlayShakeStrengthPercent",new double[]{10,11}}, {"CatchImpactShakeStrengthPercent",new double[]{8,9}},
                {"HostileOpeningShakeMaximumPercent",new double[]{15,10}}, {"TouchdownShakeStrengthPercent",new double[]{15,16}}
            };
            var lines=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')) {
                string header=Section(raw);if(header!=null)section=header;
                double[] values;string key=Key(raw);
                bool broadcast=section=="Atmosphere"||section=="Dynamic Gameday Broadcast";
                lines.Add(broadcast&&key!=null&&stock.TryGetValue(key,out values)&&Math.Abs(RealNumber(text,section,key,-1)-values[0])<0.00001
                    ?key+"="+values[1].ToString(CultureInfo.InvariantCulture):raw);
            }
            return String.Join("\n",lines.ToArray());
        }

        private static string RetuneLineup2015(string text,string defaults,string existing) {
            if(Number(existing,"General","AudioBalanceVersion",0)>=51||Number(defaults,"General","AudioBalanceVersion",0)<51)return text;
            var stock=new Dictionary<string,double[]> {
                {"OrdinaryShakeStrengthPercent",new double[]{4,5}}, {"CriticalShakeStrengthPercent",new double[]{8,10}},
                {"LineupCheerShakeStrengthPercent",new double[]{5,6}}
            };
            var lines=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')) {
                string header=Section(raw);if(header!=null)section=header;
                double[] values;string key=Key(raw);
                bool broadcast=section=="Atmosphere"||section=="Dynamic Gameday Broadcast";
                lines.Add(broadcast&&key!=null&&stock.TryGetValue(key,out values)&&Math.Abs(RealNumber(text,section,key,-1)-values[0])<0.00001
                    ?key+"="+values[1].ToString(CultureInfo.InvariantCulture):raw);
            }
            return String.Join("\n",lines.ToArray());
        }

        private static string RetuneCannon2016(string text,string defaults,string existing) {
            if(Number(existing,"General","AudioBalanceVersion",0)>=52||Number(defaults,"General","AudioBalanceVersion",0)<52)return text;
            var stock=new Dictionary<string,double[]> {
                {"CannonShakeStrengthPercent",new double[]{15,18}}
            };
            var lines=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')) {
                string header=Section(raw);if(header!=null)section=header;
                double[] values;string key=Key(raw);
                bool broadcast=section=="Atmosphere"||section=="Dynamic Gameday Broadcast";
                lines.Add(broadcast&&key!=null&&stock.TryGetValue(key,out values)&&Math.Abs(RealNumber(text,section,key,-1)-values[0])<0.00001
                    ?key+"="+values[1].ToString(CultureInfo.InvariantCulture):raw);
            }
            return String.Join("\n",lines.ToArray());
        }

        private static string RetuneHotfix2026(string text,string defaults,string existing) {
            if(Number(existing,"General","AudioBalanceVersion",0)>=56||Number(defaults,"General","AudioBalanceVersion",0)<56)return text;
            const string section="Dynamic Gameday Broadcast";
            string[] cameras={"StandardCameraShakePercent","ZoomCameraShakePercent","All22CameraShakePercent","WideCameraShakePercent","BroadcastCameraShakePercent"};
            double[] calibration={110,130,75,100,75};bool normalize=true;
            for(int i=0;i<cameras.Length;i++)if(Math.Abs(RealNumber(existing,section,cameras[i],-1)-calibration[i])>0.00001)normalize=false;
            var lines=new List<string>();string current="";
            foreach(string raw in text.Replace("\r","").Split('\n')) {
                string header=Section(raw);if(header!=null)current=header;
                string key=Key(raw),line=raw;
                if(current==section&&key=="CatchImpactShakeStrengthPercent"&&Math.Abs(RealNumber(text,section,key,-1)-9)<0.00001)line=key+"=12";
                if(current==section&&normalize&&Array.IndexOf(cameras,key)>=0)line=key+"=100";
                lines.Add(line);
            }return String.Join(Environment.NewLine,lines);
        }

        private static string RetuneHotfix2028(string text,string defaults,string original) {
            if(Number(original,"General","AudioBalanceVersion",0)>=57||Number(defaults,"General","AudioBalanceVersion",0)<57)return text;
            var lines=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')){string h=Section(raw);if(h!=null)section=h;
                lines.Add(section=="Dynamic Gameday Soundscape"&&Key(raw)=="CrowdMixStrengthPercent"&&RealNumber(text,section,"CrowdMixStrengthPercent",100)>100?"CrowdMixStrengthPercent=100":raw);
            }return String.Join(Environment.NewLine,lines);
        }
        private static string RetuneShakeSliders2030(string text,string defaults,string original) {
            if(Number(original,"General","AudioBalanceVersion",0)>=58||Number(defaults,"General","AudioBalanceVersion",0)<58)return text;
            var lines=new List<string>();string section="";
            foreach(string raw in text.Replace("\r","").Split('\n')){string h=Section(raw);if(h!=null)section=h;
                // Old On meant35%, not1%. Convert once; later1% slider choices stay1%.
                lines.Add(section=="Dynamic Gameday Broadcast"&&Key(raw)=="AwayTeamReactionShake"&&RealNumber(original,section,"AwayTeamReactionShake",0)==1?"AwayTeamReactionShake=35":raw);
            }return String.Join(Environment.NewLine,lines);
        }
        public static void Ensure(string destination,string legacy,string defaults){
            SettingsIo.Run("validate settings migration paths",legacy,destination,null,()=>{});
            string existing=File.Exists(destination)?SettingsIo.ReadText(destination):File.Exists(legacy)?SettingsIo.ReadText(legacy):defaults;
            if(Number(existing,"General","ConfigVersion",1)>Number(defaults,"General","ConfigVersion",1))return;
            bool categorized=Number(defaults,"General","ConfigVersion",1)>=2;
            if(categorized){existing=Organize(existing,false);defaults=Organize(defaults,false);}
            bool refreshHelp=((Number(existing,"General","AudioBalanceVersion",0)<59&&Number(defaults,"General","AudioBalanceVersion",0)>=59)||
                (Number(existing,"General","AttendanceBalanceVersion",0)<9&&Number(defaults,"General","AttendanceBalanceVersion",0)>=9))&&
                Number(existing,"General","ConfigVersion",1)<=Number(defaults,"General","ConfigVersion",1);
            string upgraded=RetuneOldDefaults(existing,defaults);
            upgraded=RetuneGamedayDefaults(upgraded,defaults);
            upgraded=RetuneHalftimeDefault(upgraded,defaults);
            upgraded=RetuneReleaseDefaults(upgraded,defaults);
            upgraded=RetuneEnergyDefaults(upgraded,defaults);
            upgraded=RetuneAtmosphereDefaults(upgraded,defaults);
            upgraded=RetuneOpeningDefaults(upgraded,defaults);
            upgraded=RetuneCoreAudio(upgraded,defaults);
            upgraded=RetuneReductionStrength(upgraded,defaults);
            upgraded=RetuneGentleAudio(upgraded,defaults);
            upgraded=RetunePercentageAudio(upgraded,defaults);
            upgraded=RetunePercentageMix(upgraded,defaults);
            upgraded=RetuneAttendanceDefaults(upgraded,defaults);
            upgraded=RetuneArrivalBaselines(upgraded,defaults);
            upgraded=RetuneQuarterCap(upgraded,defaults);
            upgraded=RetuneAttendanceBalance4(upgraded,defaults);
            upgraded=RetuneReplayDefaults(upgraded,defaults);
            upgraded=RetuneAttendanceAudio14(upgraded,defaults);
            upgraded=RetuneVolumeTiers15(upgraded,defaults);
            upgraded=RetuneCapacityArrivals5(upgraded,defaults);
            upgraded=RetuneRenderedAudio17(upgraded,defaults);
            upgraded=RetuneRenderedAudio18(upgraded,defaults);
            upgraded=RetuneLineupAudio19(upgraded,defaults);
            if(Number(existing,"General","ConfigVersion",1)<=Number(defaults,"General","ConfigVersion",1)&&
               Number(defaults,"Atmosphere","TouchdownShakeDurationSeconds",0)>0){
                // Replace only our former stock explanations; preserve custom help and all values.
                upgraded=upgraded.Replace("; Quick rise, then roughly three seconds of settling. Eruptions take priority.","; This is the level after the opening hit; eruptions take priority.")
                    .Replace("; The strongest added movement for an eruption, settling over about five seconds.","; Sustained eruption movement after its opening hit.")
                    .Replace("; Stronger anticipation on eligible critical downs and close scoring threats.","; Critical downs/scoring threats use this only during packed-house or big-game buildup.")
                    .Replace("; On offense or defense, an eligible lineup gets an early cheer and ~25-second roar.","; An eligible lineup builds and fades across the 40-second maximum play clock.")
                    .Replace("; Default 15. Short catches do not qualify just because the receiver runs farther afterward.","; Default 30. Immediate tackles receive an impact/fade; short catches can qualify at the total-gain cutoff.")
                    .Replace("; Fourth down needs a verified offensive go-for-it play; punts/field goals do not shake.","; Critical fourth-down shake needs an offensive go-for-it play. Field goals/punts may use ordinary shake; PATs and other kicks do not. Big-game opening kickoffs carry through the play.");
            }
            if(Number(existing,"General","AudioBalanceVersion",0)<20&&Number(defaults,"General","AudioBalanceVersion",0)>=20){
                // Retune the former stock cutoff; keep deliberate custom thresholds.
                var cutoffLines=new List<string>(upgraded.Replace("\r","").Split('\n'));string cutoffSection="";
                for(int i=0;i<cutoffLines.Count;i++){string next=Section(cutoffLines[i]);if(next!=null){cutoffSection=next;continue;}
                    if(cutoffSection=="Dynamic Gameday Soundscape"&&Key(cutoffLines[i])=="LongCompletionMinimumYards"&&Number(upgraded,cutoffSection,"LongCompletionMinimumYards",0)==15)cutoffLines[i]="LongCompletionMinimumYards=30";
                    if(cutoffSection=="General"&&Key(cutoffLines[i])=="AudioBalanceVersion")cutoffLines[i]="AudioBalanceVersion=20";
                }
                upgraded=String.Join(Environment.NewLine,cutoffLines);
            }
            upgraded=Retune1380(upgraded,defaults,existing);
            upgraded=Retune1390(upgraded,defaults,existing);
            upgraded=Retune1400(upgraded,defaults,existing);
            upgraded=Retune1410(upgraded,defaults,existing);
            upgraded=Retune1430(upgraded,defaults);
            upgraded=Retune1431(upgraded,defaults,existing);
            upgraded=Retune1470(upgraded,defaults,existing);
            upgraded=Retune1500(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,39);
            upgraded=Retune1558(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,40);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,42);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,43);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,44);
            upgraded=Retune1940(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,45);
            upgraded=RetuneBusMixDefault(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,47);
            upgraded=RetuneShake2030(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,48);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,49);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,50);
            upgraded=RetuneLineup2015(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,51);
            upgraded=RetuneCannon2016(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,52);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,53);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,54);
            upgraded=Retune2020(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,55);
            upgraded=RetuneHotfix2026(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,56);
            upgraded=RetuneHotfix2028(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,57);
            upgraded=RetuneShakeSliders2030(upgraded,defaults,existing);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,58);
            upgraded=AdvanceHelpVersion(upgraded,defaults,existing,59);
            upgraded=upgraded.Replace("; Ordinary matchups keep their usual packed-house requirement.",
                "; Ordinary opening drives use critical-down fullness: 90% in small venues to 55% in the largest."+Environment.NewLine+
                "; Ends with the opening possession. Later ordinary drives use their normal requirements.");
            upgraded=upgraded.Replace("; Default 4.5. Touchdowns and eruptions take over using their separate profiles.",
                "; Default 4.5 includes a brief hold, then a smooth fade. TDs/eruptions use separate profiles.")
                .Replace("; Live runs/returns keep the separate post-whistle fade above.",
                "; Short impacts hit quickly, hold briefly, then fade. Live runs/returns use the settling time above.");
            upgraded=RetuneAttendance1820(upgraded,defaults,existing);
            upgraded=RetuneAttendance1830(upgraded,defaults,existing);
            string result=Merge(upgraded,defaults);
            if(refreshHelp)result=RefreshHelp(result,defaults);
            if(categorized){
                var lines=new List<string>(result.Replace("\r","").Split('\n'));string currentSection="";
                for(int i=0;i<lines.Count;i++){string next=Section(lines[i]);if(next!=null){currentSection=next;continue;}if(currentSection=="General"&&Key(lines[i])=="ConfigVersion")lines[i]="ConfigVersion=2";}
                result=Organize(String.Join(Environment.NewLine,lines),true);
            }
            if(File.Exists(destination)&&SettingsIo.ReadText(destination)==result)return;
            // Win32 profile APIs reliably support UTF-16 BOM settings files.
            string temp=destination+".tmp";Exception failure=null;
            try{
                SettingsIo.Run("write migrated temporary settings",null,temp,null,()=>File.WriteAllText(temp,result,Encoding.Unicode));
                if(File.Exists(destination))SettingsIo.Run("replace migrated settings",temp,destination,destination+".previous",()=>File.Replace(temp,destination,destination+".previous",true));
                else SettingsIo.Run("activate new settings",temp,destination,null,()=>File.Move(temp,destination));
            }catch(Exception error){failure=error;throw;}
            finally{SettingsIo.Cleanup(temp,failure);}
        }
    }
}
