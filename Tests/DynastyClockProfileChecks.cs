using System;
using System.Collections.Generic;
using System.IO;
using OfflineDynastySettings;

internal static class DynastyClockProfileChecks {
    static int checks;
    static void Check(bool condition,string name){if(!condition)throw new Exception(name);checks++;}
    static void Reject(Action action,string name){try{action();}catch(InvalidDataException){checks++;return;}throw new Exception("Accepted "+name);}
    static int Main(string[] args) {
        string root=Path.GetFullPath(args[0]);Directory.CreateDirectory(root);typeof(Ace.DynastyCOT.UserStorage).GetField("current",System.Reflection.BindingFlags.NonPublic|System.Reflection.BindingFlags.Static).SetValue(null,new Ace.DynastyCOT.UserStore(Path.Combine(root,"Ace","DynastyCOT")));
        string folder=Path.Combine(root,Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(folder);
        // Legacy global choices deliberately do not seed per-Dynasty defaults.
        File.WriteAllText(Path.Combine(folder,"DynastyPlayClock.ini"),"ExtendedSeconds=60");
        File.WriteAllText(Path.Combine(folder,"OfflineDynastySettings.ini"),"DefensivePlayClockSeconds=60");
        Check(DynastyClockProfiles.Load(folder).Count==0,"global settings leaked");
        Check(DynastyClockProfiles.Default.OffensiveMode==1 && DynastyClockProfiles.Default.DefensiveSeconds==15,"defaults");
        var values=new Dictionary<uint,DynastyClockValue>();
        int[] defenses={0,3,5,15,20,30,45,60};uint league=1;
        for(int offense=0;offense<=3;offense++)foreach(int defense in defenses)values.Add(league++,new DynastyClockValue(offense,defense));
        values.Add(0x80000001,new DynastyClockValue(3,5));
        DynastyClockProfiles.Save(folder,values);
        var restored=DynastyClockProfiles.Load(folder);
        foreach(var item in values)Check(restored[item.Key].OffensiveMode==item.Value.OffensiveMode
            && restored[item.Key].DefensiveSeconds==item.Value.DefensiveSeconds,"round trip "+item.Key);
        var memory=DynastyClockProfiles.Encode(values);var decoded=DynastyClockProfiles.Decode(memory);
        foreach(var item in values)Check(decoded[item.Key].OffensiveMode==item.Value.OffensiveMode
            && decoded[item.Key].DefensiveSeconds==item.Value.DefensiveSeconds,"native snapshot "+item.Key);
        // Updating B leaves A and every other Dynasty intact, including reload.
        restored[2]=new DynastyClockValue(0,0);DynastyClockProfiles.Save(folder,restored);
        var switched=DynastyClockProfiles.Load(folder);
        Check(switched[1].OffensiveMode==values[1].OffensiveMode && switched[1].DefensiveSeconds==values[1].DefensiveSeconds,"A overwritten");
        Check(switched[2].OffensiveMode==0 && switched[2].DefensiveSeconds==0,"B not persisted");
        Check(File.Exists(DynastyClockProfiles.FilePath(folder)+".bak"),"previous document backup");
        Reject(()=>new DynastyClockValue(4,15),"invalid offense");
        Reject(()=>new DynastyClockValue(1,4),"invalid defense");
        Reject(()=>DynastyClockProfiles.Encode(new Dictionary<uint,DynastyClockValue>{{0,DynastyClockProfiles.Default}}),"zero identity");
        Reject(()=>DynastyClockProfiles.Encode(new Dictionary<uint,DynastyClockValue>{{UInt32.MaxValue,DynastyClockProfiles.Default}}),"sentinel identity");
        byte[] bad=(byte[])memory.Clone();bad[7]=1;Reject(()=>DynastyClockProfiles.Decode(bad),"reserved bits");
        bad=(byte[])memory.Clone();Array.Copy(bad,0,bad,8,8);Reject(()=>DynastyClockProfiles.Decode(bad),"duplicate identity");
        bad=(byte[])memory.Clone();Array.Clear(bad,0,8);Reject(()=>DynastyClockProfiles.Decode(bad),"table hole");
        string path=DynastyClockProfiles.FilePath(folder);
        foreach(string invalid in new[]{"{}","{\"format\":2,\"dynasties\":{}}","{\"format\":1,\"dynasties\":{\"01\":{\"offensiveMode\":1,\"defensiveSeconds\":15}}}",
            "{\"format\":1,\"dynasties\":{\"1\":{\"offensiveMode\":\"1\",\"defensiveSeconds\":15}}}"}) {
            File.WriteAllText(path,invalid);Reject(()=>DynastyClockProfiles.Load(folder),"invalid file");
            Check(File.ReadAllText(path)==invalid,"invalid file overwritten");
        }
        Console.WriteLine("PASS: "+checks+" per-Dynasty clock profile checks; all 32 clock combinations, unsigned identity, isolation, reload, invalid inputs and backups.");
        return 0;
    }
}
