// Released-installer migration harness. File operations only, marked MMC copies.
using System;using System.IO;using System.Linq;using System.Reflection;using System.Collections.Generic;using System.Threading;using DynastyCOT.Collection;
class CollectionReleasedMigration {
 static int checks;static void Check(bool ok,string name){if(!ok)throw new IOException(name);++checks;}
 static object Call(Type type,string name,params object[] args){try{return type.GetMethod(name,BindingFlags.Static|BindingFlags.Public|BindingFlags.NonPublic).Invoke(null,args);}catch(TargetInvocationException e){throw e.InnerException;}}
 static int Main(string[] args){try{
  typeof(Ace.DynastyCOT.UserStorage).GetField("current",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,new Ace.DynastyCOT.UserStore(Path.Combine(args[3],"UserData")));
  string mode=args[0],source=Path.GetFullPath(args[1]),root=Paths.ValidateRoot(args[2]),parent=Path.GetFullPath(args[3]),sdk=Path.GetFullPath(args[4]);var fixture=new Fixture(root,parent);var catalog=Json.Read<Catalog>(Path.Combine(source,"Packaging/Collection/catalog.json"));var engine=new CollectionInstaller(catalog,fixture);
  AppDomain.CurrentDomain.AssemblyResolve+=(s,e)=>{string file=Path.Combine(sdk,new AssemblyName(e.Name).Name+".dll");return File.Exists(file)?Assembly.LoadFrom(file):null;};
  string state=Paths.Resolve(root,".migration-test.json");
  if(mode=="migrate"){
   var inv=engine.Inspect(root);Check(inv.LegacyHost,"Actual old COT host not recognized");var enabled=catalog.Modules.Where(m=>inv.LegacyEnabled[m.Id]).ToArray();Check(enabled.Length>0,"No enabled old modules");
   var client=new Downloads(fixture,(url,limit)=>{string file=Path.Combine(args[5],Path.GetFileName(new Uri(url).AbsolutePath));return File.ReadAllBytes(file);});var packs=enabled.Select(m=>client.Prepare(m,m.Current,Path.Combine(parent,"DownloadCache"),CancellationToken.None,s=>{})).ToList();var plan=engine.Install(root,packs.ToDictionary(x=>x.Module.Id,x=>x.Release),packs);
   Check(engine.Review(plan).Count==0,"Released old host unexpectedly requires unrecognized-file approval");string backup=engine.Execute(plan,new Dictionary<string,string>(),s=>{});File.WriteAllBytes(state,Json.Bytes(new{backup=backup,enabled=enabled.Select(x=>x.Id).ToArray()}));
   Check(!File.Exists(Paths.Resolve(root,"Plugins/DynastyCOT.dll")),"Old host remains in scan");Check(engine.Inspect(root).Plugins.Count==enabled.Length,"Wrong number of active feature providers");foreach(var m in enabled)Check(Paths.HashFile(Paths.Resolve(root,"Plugins/"+m.FileName))==m.Current.DllSha256,"Standalone differs from released DLL");
  }else if(mode=="prepare"){
   var doc=Json.Read<Dictionary<string,object>>(state);foreach(string id in Json.Array(doc["enabled"]).Cast<string>()){
    var module=catalog.Get(id);var assembly=Assembly.LoadFile(Paths.Resolve(root,"Plugins/"+module.FileName));var user=assembly.GetType("Ace.DynastyCOT.UserStorage",true);var store=Activator.CreateInstance(assembly.GetType("Ace.DynastyCOT.UserStore",true),new object[]{Path.Combine(parent,"UserData")});user.GetField("current",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,store);
    if(id=="GamedayAtmosphere"){
     var action=assembly.GetType("DynamicGamedayAtmosphere.AttendanceAction",true);var resource=assembly.GetManifestResourceNames().Single(x=>x.StartsWith("Attendance.GamedayAtmosphereNative-")&&x.EndsWith(".dll"));string file=resource.Substring("Attendance.".Length),folder=(string)action.GetProperty("RuntimeFolder",BindingFlags.Static|BindingFlags.Public|BindingFlags.NonPublic).GetValue(null,null);Check(Path.GetFullPath(folder).StartsWith(Path.Combine(parent,"UserData")+"\\",StringComparison.OrdinalIgnoreCase),"GDA runtime left the isolated AppData fixture");Call(action,"Extract",file,false);Check(File.Exists(Path.Combine(folder,file)),"GDA materialization failed");
    }else{
     var runtime=assembly.GetType("DynastyCOTStandalone.Runtime",true);Check(Call(runtime,"Conflict")==null,"Old-host/standalone conflict after migration");Call(assembly.GetType("Ace.DynastyCOT.UserStorage",true),"Migrate",module.LegacyId,Path.Combine(root,"Plugins"));Call(runtime,"Prepare");Dictionary<string,string> manifest;using(var reader=new StreamReader(assembly.GetManifestResourceStream("Standalone.payload.json")))manifest=Json.Serializer().Deserialize<Dictionary<string,string>>(reader.ReadToEnd());foreach(var item in manifest)Check(Paths.HashFile((string)Call(runtime,"Resolve",item.Key))==item.Value,"Cold support-file materialization differs: "+item.Key);
    }
   }
  }else if(mode=="restore"){
   var doc=Json.Read<Dictionary<string,object>>(state);engine.Restore(root,(string)doc["backup"]);Check(engine.Inspect(root).LegacyHost,"Old host was not restored");foreach(var m in catalog.Modules)Check(!File.Exists(Paths.Resolve(root,"Plugins/"+m.FileName)),"Duplicate standalone after old-host restore");
  }else if(mode=="verify-host"){
   var assembly=Assembly.LoadFile(Paths.Resolve(root,"Plugins/DynastyCOT.dll"));var action=assembly.GetType("DynastyCOT.LaunchAction",true);var ids=new HashSet<string>();using(var reader=new StreamReader(assembly.GetManifestResourceStream("DynastyCOT.payload.txt")))for(string line;(line=reader.ReadLine())!=null;)if(line.Length>0)ids.Add(line.Split('|')[0]);foreach(string id in ids.Where(x=>x!="Core")){Call(action,"VerifyModule",id);++checks;}
  }else if(mode=="remove-restore"){
   var inv=engine.Inspect(root);var chosen=inv.Plugins.First(x=>x.Module!="Core");string before=chosen.Sha256;string backup=engine.Execute(engine.Remove(root,new[]{chosen.Module}),new Dictionary<string,string>(),s=>{});Check(!File.Exists(Paths.Resolve(root,chosen.Path)),"Selected removal failed");engine.Restore(root,backup);Check(Paths.HashFile(Paths.Resolve(root,chosen.Path))==before,"Selected removal rollback failed");
  }else throw new IOException("Unknown migration mode.");
  Console.WriteLine("PASS "+checks+" released migration "+mode);return 0;
 }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
}
