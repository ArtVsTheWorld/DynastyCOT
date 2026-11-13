using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Web.Script.Serialization;

namespace OfflineDynastySettings {
    internal sealed class DynastyClockValue {
        public int OffensiveMode { get; private set; }
        public int DefensiveSeconds { get; private set; }
        internal DynastyClockValue(int offense,int defense) {
            if(offense<0 || offense>3 || !ClockPreferences.Valid(defense))throw new InvalidDataException("Invalid Dynasty clock choice.");
            OffensiveMode=offense;DefensiveSeconds=defense;
        }
    }
    // Persistent keys use the same normalized unsigned LeagueID as playcall settings.
    // No global INI is read: a new Dynasty always starts On / 15 seconds.
    internal static class DynastyClockProfiles {
        internal const int Capacity=1024, SequenceOffset=0x1000, LockOffset=0x1004, TableOffset=0x1100;
        internal const int AllocationSize=16384;
        internal static bool ValidLeague(uint league) {return league!=0 && league!=UInt32.MaxValue;}
        internal static string FilePath(string plugins) {
            return Path.Combine(Ace.DynastyCOT.UserStorage.Folder("ExtraSettings"),"DynastyClocks.json");
        }
        internal static DynastyClockValue Default {get{return new DynastyClockValue(1,15);}}
        private static int Integer(Dictionary<string,object> data,string key) {
            object value;
            if(!data.TryGetValue(key,out value) || !(value is int))throw new InvalidDataException("Missing or invalid clock field: "+key);
            return (int)value;
        }
        internal static Dictionary<uint,DynastyClockValue> Load(string plugins) {
            var result=new Dictionary<uint,DynastyClockValue>();string path=FilePath(plugins);
            if(!File.Exists(path))return result;
            var root=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(path)) as Dictionary<string,object>;
            if(root==null || Integer(root,"format")!=1)throw new InvalidDataException("Unsupported Dynasty clock format; file preserved.");
            object value;Dictionary<string,object> records;
            if(!root.TryGetValue("dynasties",out value) || (records=value as Dictionary<string,object>)==null || records.Count>Capacity)
                throw new InvalidDataException("Invalid Dynasty clock profiles; file preserved.");
            foreach(var item in records) {
                uint league;var fields=item.Value as Dictionary<string,object>;
                if(!UInt32.TryParse(item.Key,NumberStyles.None,CultureInfo.InvariantCulture,out league) || !ValidLeague(league)
                    || league.ToString(CultureInfo.InvariantCulture)!=item.Key || fields==null)
                    throw new InvalidDataException("Invalid Dynasty clock identity; file preserved.");
                result.Add(league,new DynastyClockValue(Integer(fields,"offensiveMode"),Integer(fields,"defensiveSeconds")));
            }
            return result;
        }
        internal static Dictionary<uint,DynastyClockValue> Decode(byte[] bytes) {
            if(bytes==null || bytes.Length!=Capacity*8)throw new InvalidDataException("Incomplete Dynasty clock snapshot.");
            var result=new Dictionary<uint,DynastyClockValue>();bool empty=false;
            for(int i=0;i<Capacity;i++) {
                ulong record=BitConverter.ToUInt64(bytes,i*8);
                if(record==0){empty=true;continue;}
                uint league=(uint)record;
                if(empty || !ValidLeague(league) || record>>48!=0 || result.ContainsKey(league))
                    throw new InvalidDataException("Invalid or changing Dynasty clock table; saved profiles preserved.");
                result.Add(league,new DynastyClockValue((int)((record>>32)&255),(int)((record>>40)&255)));
            }
            return result;
        }
        internal static byte[] Encode(Dictionary<uint,DynastyClockValue> values) {
            if(values==null || values.Count>Capacity)throw new InvalidDataException("Too many Dynasty clock profiles.");
            var bytes=new byte[Capacity*8];var keys=new List<uint>(values.Keys);keys.Sort();int index=0;
            foreach(uint league in keys) {
                if(!ValidLeague(league) || values[league]==null)throw new InvalidDataException("Invalid clock identity.");
                var choice=values[league];new DynastyClockValue(choice.OffensiveMode,choice.DefensiveSeconds);
                ulong packed=league|((ulong)choice.OffensiveMode<<32)|((ulong)choice.DefensiveSeconds<<40);
                Array.Copy(BitConverter.GetBytes(packed),0,bytes,index++*8,8);
            }
            return bytes;
        }
        internal static void Save(string plugins,Dictionary<uint,DynastyClockValue> values) {
            // Validate the complete snapshot before preparing a write.
            Encode(values);
            var records=new SortedDictionary<string,object>(StringComparer.Ordinal);
            foreach(var item in values)records.Add(item.Key.ToString(CultureInfo.InvariantCulture),
                new Dictionary<string,object>{{"offensiveMode",item.Value.OffensiveMode},{"defensiveSeconds",item.Value.DefensiveSeconds}});
            string path=FilePath(plugins),temp=path+"."+Guid.NewGuid().ToString("N")+".tmp";
            Directory.CreateDirectory(Path.GetDirectoryName(path));
            try {
                using(var file=new FileStream(temp,FileMode.CreateNew,FileAccess.Write,FileShare.None))
                using(var writer=new StreamWriter(file)) {
                    writer.Write(new JavaScriptSerializer().Serialize(new Dictionary<string,object>{{"format",1},{"dynasties",records}}));
                    writer.Flush();file.Flush(true);
                }
                if(File.Exists(path))File.Replace(temp,path,path+".bak");else File.Move(temp,path);
            } finally {if(File.Exists(temp))File.Delete(temp);}
        }
    }
}
