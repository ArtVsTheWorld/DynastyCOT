using System;
using System.Collections.Generic;
using System.IO;
using OfflineDynastySettings;

class CoreChecks
{
    static int checks;
    static void Check(bool value,string message) { if(!value)throw new Exception(message);checks++; }
    sealed class Session : ILaunchSession
    {
        internal int Clocks,Splits,Polls,Verifies,Disposals;
        internal bool Available=true,Changed;
        public string InstallClock(){Clocks++;return Available?"Installed test hooks":"Waiting for decrypted code";}
        public bool InstallPlaycall(){Splits++;return Available;}
        public void Poll(){Polls++;}
        public void Verify(){Verifies++;if(Changed)throw new HookOwnershipException("test changed bytes");}
        public void Dispose(){Disposals++;}
    }
    sealed class Env : ILaunchEnvironment
    {
        internal DateTime Clock=new DateTime(2026,1,1,0,0,0,DateTimeKind.Utc);
        internal GameLaunch Game;
        internal Session Current;
        internal List<string> Messages=new List<string>();
        public DateTime Now{get{return Clock;}}
        public GameLaunch FindGame(){return Game;}
        public bool IsAlive(GameLaunch game){return Game!=null && Game.Same(game);}
        public ILaunchSession Begin(GameLaunch game){return Current=new Session();}
        public void Log(string message){Messages.Add(message);}
        internal void Launch(int pid){Game=new GameLaunch{Pid=pid,Started=Clock.Ticks,StartedUtc=Clock};}
    }
    static void Main(string[] args)
    {
        if(args.Length!=2||args[1]!="--isolated")throw new InvalidOperationException("Use a marked fixture and --isolated; this legacy harness must never use shared live settings.");
        string folder=Path.GetFullPath(args[0]);
        if(!File.Exists(Path.Combine(folder,".cot-fixture")))throw new InvalidOperationException("Marked fixture required before any settings access.");
        typeof(Ace.DynastyCOT.UserStorage).GetField("current",System.Reflection.BindingFlags.Static|System.Reflection.BindingFlags.NonPublic).SetValue(null,new Ace.DynastyCOT.UserStore(Path.Combine(folder,"UserData")));
        Directory.CreateDirectory(folder);
        string ini=SettingsFiles.PathFor(folder,ClockPreferences.FileName);
        if(File.Exists(ini))File.Delete(ini);
        Check(ClockPreferences.Load(folder)==15,"New installations default to 15 seconds");
        Check(!File.Exists(ini),"Reading defaults must not write settings");
        foreach(int value in new[]{3,5,15,20,30,45,60,0}){
            ClockPreferences.Save(folder,value);
            Check(ClockPreferences.Load(folder)==value,"Clock preference did not survive reload");
        }
        string saved=File.ReadAllText(ini);
        foreach(int value in new[]{-1,1,14,16,59,61,999}){
            bool rejected=false;try{ClockPreferences.Save(folder,value);}catch(InvalidDataException){rejected=true;}
            Check(rejected && File.ReadAllText(ini)==saved,"Invalid value overwrote a saved choice");
        }
        File.WriteAllText(ini,"DefensivePlayClockSeconds=invalid");
        bool corrupt=false;try{ClockPreferences.Load(folder);}catch(InvalidDataException){corrupt=true;}
        Check(corrupt && File.ReadAllText(ini).Contains("invalid"),"Corrupt preferences must be preserved");
        // Current COT storage copies legacy data and keeps originals. The
        // older move/delete migration assertions no longer describe production.
        File.Delete(ini);
        string migration=Path.Combine(folder,"LegacyMMC","Plugins");Directory.CreateDirectory(migration);
        string legacyIni=Path.Combine(migration,ClockPreferences.FileName);File.WriteAllText(legacyIni,"DefensivePlayClockSeconds=45\r\n");
        File.WriteAllText(Path.Combine(migration,"AnotherPlugin.ini"),"untouched");
        SettingsFiles.Migrate(migration);
        Check(ClockPreferences.Load(migration)==45,"Legacy preference copied to isolated shared data");
        Check(File.ReadAllText(legacyIni)=="DefensivePlayClockSeconds=45\r\n","Original legacy preference retained");
        ClockPreferences.Save(folder,20);SettingsFiles.Migrate(migration);
        Check(ClockPreferences.Load(folder)==20,"Completed migration never overwrites current preferences");
        Check(File.ReadAllText(Path.Combine(migration,"AnotherPlugin.ini"))=="untouched","Unrelated data retained");
        Env env=new Env();
        using(LaunchCoordinator coordinator=new LaunchCoordinator(env)){
            coordinator.Step();Check(env.Current==null,"Do not activate without a game");
            env.Launch(100);coordinator.Step();Check(env.Current==null,"Respect startup grace");
            env.Clock=env.Clock.AddSeconds(46);coordinator.Step();Session first=env.Current;
            Check(first.Clocks==1 && first.Splits==1 && first.Verifies==1 && first.Polls==1,"Both components must activate and verify");
            env.Clock=env.Clock.AddSeconds(6);coordinator.Step();
            Check(first.Clocks==1 && first.Splits==1 && first.Verifies==2,"Do not reinstall owned hooks");
            first.Changed=true;env.Clock=env.Clock.AddSeconds(6);coordinator.Step();int before=first.Polls;
            env.Clock=env.Clock.AddSeconds(6);coordinator.Step();
            Check(first.Polls==before,"Stop this launch after an ownership change");
            env.Launch(200);coordinator.Step();Check(first.Disposals==1,"Dispose the previous launch");
            env.Clock=env.Clock.AddSeconds(46);coordinator.Step();
            Check(env.Current!=first && env.Current.Verifies==1,"A fresh game can activate after a previous conflict");
            Session second=env.Current;env.Game=null;coordinator.Step();
            Check(second.Disposals==1,"Release the session after the game exits");
        }
        Console.WriteLine("PASS "+checks+" preference and launch-coordinator checks (no game or MMC required)");
    }
}
