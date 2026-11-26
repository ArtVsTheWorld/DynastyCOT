// Offline fixtures only: never opens the game or starts the plugin watcher.
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Web.Script.Serialization;
class PracticeInjuryChecks {
    static int checks;
    static int failAt,writes;static bool rollbackFailure;
    static Dictionary<long,byte> memory=new Dictionary<long,byte>();
    static void FakeWrite<T>(T change,bool restore){
        var type=typeof(T);var flags=BindingFlags.Instance|BindingFlags.NonPublic;
        long address=(long)type.GetField("Address",flags).GetValue(change);
        byte[] bytes=(byte[])type.GetField(restore?"Before":"After",flags).GetValue(change);
        if(restore&&rollbackFailure)throw new IOException("mock rollback lock");
        memory[address]=bytes[0];
        if(!restore&&++writes==failAt)throw new IOException("mock interrupted write/readback");
    }
    static void Check(bool yes,string why){if(!yes)throw new Exception(why);checks++;}
    static object Call(Type type,string method,params object[] args){return type.GetMethod(method,BindingFlags.NonPublic|BindingFlags.Static).Invoke(null,args);}
    static int Main(string[] args){try{
        string dllPath=Path.GetFullPath(args[0]),sdk=Path.GetFullPath(args[1]),root=Path.GetFullPath(args[2]);Directory.CreateDirectory(root);
        AppDomain.CurrentDomain.AssemblyResolve+=(s,e)=>{string p=Path.Combine(sdk,new AssemblyName(e.Name).Name+".dll");return File.Exists(p)?Assembly.LoadFrom(p):null;};
        Assembly dll=Assembly.LoadFrom(dllPath);BindingFlags hidden=BindingFlags.NonPublic|BindingFlags.Static;
        dll.GetType("Ace.DynastyCOT.UserStorage").GetField("current",hidden).SetValue(null,
            Activator.CreateInstance(dll.GetType("Ace.DynastyCOT.UserStore"),new object[]{Path.Combine(root,"UserData")}));
        Type profiles=dll.GetType("OfflineDynastySettings.DynastyClockProfiles"),clock=dll.GetType("DynastyPlayClock.DynastyPlayClockAction");
        string path=(string)Call(profiles,"FilePath",root);Directory.CreateDirectory(Path.GetDirectoryName(path));
        const string original="{\"format\":1,\"dynasties\":{\"101\":{\"offensiveMode\":3,\"defensiveSeconds\":5},\"202\":{\"offensiveMode\":0,\"defensiveSeconds\":0}}}";
        File.WriteAllText(path,original);
        IDictionary loaded=(IDictionary)Call(profiles,"Load",root);
        Check((int)loaded[(uint)101].GetType().GetProperty("PracticeInjuryMode").GetValue(loaded[(uint)101],null)==0,"Legacy defaults to native injuries");
        byte[] encoded=(byte[])Call(profiles,"Encode",loaded);
        Check(BitConverter.ToUInt64(encoded,0)==(101UL|(3UL<<32)|(5UL<<40)),"Legacy clock pair preserved exactly");
        Check(File.ReadAllText(path)==original,"Read-only migration leaves saved file byte-identical");
        foreach(string bad in new[]{"-1","3","1.5","null","\"1\""}){
            string invalid=original.Replace("\"defensiveSeconds\":5","\"defensiveSeconds\":5,\"practiceInjuryMode\":"+bad);
            File.WriteAllText(path,invalid);bool rejected=false;
            try{Call(profiles,"Load",root);}catch(TargetInvocationException ex){rejected=ex.InnerException is InvalidDataException;}
            Check(rejected&&File.ReadAllText(path)==invalid,"Invalid preference preserved: "+bad);
        }
        File.WriteAllText(path,original);
        for(int mode=0;mode<=2;mode++){
            byte[] table=(byte[])encoded.Clone();ulong record=BitConverter.ToUInt64(table,0)|((ulong)mode<<48);
            Array.Copy(BitConverter.GetBytes(record),table,8);object values=Call(profiles,"Decode",table);
            Call(profiles,"Save",root,values);Check(File.Exists(path+".bak"),"Atomic backup retained");
            byte[] round=(byte[])Call(profiles,"Encode",Call(profiles,"Load",root));
            Check(BitConverter.ToUInt64(round,0)==record&&BitConverter.ToUInt64(round,8)==202,"Preference and both clock pairs round-trip: "+mode);
        }
        string saved=File.ReadAllText(path);
        foreach(ulong bad in new[]{101UL|(3UL<<48),101UL|(1UL<<56),0xffffffffUL|(1UL<<48)}){
            byte[] table=(byte[])encoded.Clone();Array.Copy(BitConverter.GetBytes(bad),table,8);bool rejected=false;
            try{Call(profiles,"Decode",table);}catch(TargetInvocationException ex){rejected=ex.InnerException is InvalidDataException;}
            Check(rejected&&File.ReadAllText(path)==saved,"Invalid/changing native record does not overwrite saved settings");
        }
        long image=0x140000000L,data=0x220000000L;
        byte[] descriptor=(byte[])Call(clock,"BuildData",data);
        Check(descriptor.Length==0x4000&&BitConverter.ToInt64(descriptor,0x3780)==0&&BitConverter.ToInt64(descriptor,0x3788)==0,"Feature stays hidden until verified installation publishes callbacks");
        for(int mode=0;mode<=2;mode++){
            int option=0x3380+mode*0x80;Check(BitConverter.ToInt64(descriptor,0x3300+8*mode)==data+option&&BitConverter.ToInt32(descriptor,option+0x28)==mode,"Choice object "+mode);
        }
        Check(BitConverter.ToInt64(descriptor,0x3280)==data+0x3318,"Exactly three choices");
        string[] labels={"Default","Off for QBs","Off"};
        for(int i=0;i<3;i++)Check(System.Text.Encoding.ASCII.GetString(descriptor,0x36c0+i*0x40,labels[i].Length)==labels[i],"Label "+i);
        byte[] writer=(byte[])Call(clock,"BuildWriterUpdated",image,data);Check(writer.Length<1024,"Clock writer stays inside owned 1 KiB slot");
        var exports=new Dictionary<string,object>();
        for(int i=0;i<2;i++){
            byte[] body=(byte[])Call(clock,"BuildPracticeInjuryBridge",i,image,data);Check(body.Length<=0x800,"Bridge bounded");exports.Add("bridge"+i,Convert.ToBase64String(body));
        }
        exports.Add("read",Convert.ToBase64String((byte[])Call(clock,"ExportPracticeRead",image,data)));
        exports.Add("write",Convert.ToBase64String((byte[])Call(clock,"ExportPracticeWrite",image,data)));
        exports.Add("data",Convert.ToBase64String(descriptor));
        File.WriteAllText(Path.Combine(root,"exports.json"),new JavaScriptSerializer().Serialize(exports));
        Type changeType=clock.GetNestedType("ControllerChange",BindingFlags.NonPublic);
        IList changes=(IList)Activator.CreateInstance(typeof(List<>).MakeGenericType(changeType));
        for(int i=0;i<4;i++)changes.Add(changeType.GetConstructors(BindingFlags.NonPublic|BindingFlags.Instance)[0].Invoke(new object[]{(long)i,new byte[]{1},new byte[]{2}}));
        Delegate apply=Delegate.CreateDelegate(typeof(Action<,>).MakeGenericType(changeType,typeof(bool)),typeof(PracticeInjuryChecks).GetMethod("FakeWrite",hidden).MakeGenericMethod(changeType));
        for(int fail=1;fail<=4;fail++){
            for(int i=0;i<4;i++)memory[i]=1;failAt=fail;writes=0;rollbackFailure=false;bool rejected=false;
            try{Call(clock,"CommitPracticeChanges",changes,apply);}catch(TargetInvocationException ex){rejected=ex.InnerException is IOException;}
            Check(rejected,"Interrupted transaction reported");for(int i=0;i<4;i++)Check(memory[i]==1,"Rollback restored entry/pointer "+fail+"/"+i);
        }
        failAt=2;writes=0;rollbackFailure=true;bool unresolved=false;
        try{Call(clock,"CommitPracticeChanges",changes,apply);}catch(TargetInvocationException ex){unresolved=ex.InnerException.Message.Contains("rollback was incomplete");}
        Check(unresolved,"Incomplete rollback is explicit");
        Console.WriteLine("PASS "+checks+" practice preference, legacy migration, isolated storage, descriptor and native code-bound checks. No game access.");return 0;
    }catch(Exception ex){Console.Error.WriteLine(ex);return 1;}}
}
