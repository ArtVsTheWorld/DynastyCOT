using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Web.Script.Serialization;

namespace DynastyCOT.Collection {
 internal sealed class CollectionInstaller {
  internal readonly Catalog Catalog;
  readonly Fixture fixture;
  internal CollectionInstaller(Catalog catalog,Fixture test=null){catalog.Validate();Catalog=catalog;fixture=test;}
  internal Inventory Inspect(string value){
   string root=Paths.ValidateRoot(value);var result=new Inventory();string receipt=Paths.Resolve(root,Paths.State);
   if(File.Exists(receipt)){result.Receipt=Json.Read<Receipt>(receipt);if(result.Receipt.Format!=1||result.Receipt.Modules==null||result.Receipt.Modules.Select(x=>x.Module).Distinct().Count()!=result.Receipt.Modules.Count)throw new IOException("Collection receipt is unreadable; no files were changed.");foreach(var m in result.Receipt.Modules){Catalog.Get(m.Module);ReleaseRules.Hash(m.Sha256);ReleaseRules.Numeric(m.Version);}}
   var todo=new Stack<string>();string plugins=Paths.Resolve(root,"Plugins");if(Directory.Exists(plugins))todo.Push(plugins);int files=0;
   while(todo.Count>0){string folder=todo.Pop();Paths.NoLinks(root,folder);foreach(string child in Directory.GetDirectories(folder)){Paths.NoLinks(root,child);todo.Push(child);}foreach(string file in Directory.GetFiles(folder,"*.dll")){
    if(++files>10000)throw new IOException("Plugins scan exceeds its limit.");Paths.NoLinks(root,file);string relative=file.Substring(root.Length+1).Replace('\\','/'),name=Path.GetFileNameWithoutExtension(file),assembly=null;
    try{assembly=AssemblyName.GetAssemblyName(file).Name;}catch(BadImageFormatException){}catch(FileLoadException){}
    string owner=name.Equals("DynastyCOT",StringComparison.OrdinalIgnoreCase)||assembly=="DynastyCOT"?"Core":null;
    foreach(var m in Catalog.Modules)if(m.Aliases.Concat(new[]{m.Id}).Any(x=>String.Equals(x,name,StringComparison.OrdinalIgnoreCase)||String.Equals(x,assembly,StringComparison.OrdinalIgnoreCase)))owner=m.Id;
    if(new[]{"BeyondTheFieldNative","BeyondTheFieldRecruitFog","DynastyEnhancerNative","DynastyEnhancerRecruitFog","PlayerAnalyticsNative"}.Any(x=>name.StartsWith(x,StringComparison.OrdinalIgnoreCase)))owner="BeyondTheField";
    if(new[]{"GamedayAtmosphereNative","DynamicGamedayAtmosphereNative","DynamicAttendanceNative"}.Any(x=>name.StartsWith(x,StringComparison.OrdinalIgnoreCase)))owner="GamedayAtmosphere";
    var known=Catalog.KnownPlugins.FirstOrDefault(x=>x.Path.Equals(relative,StringComparison.OrdinalIgnoreCase));if(owner==null&&known!=null)owner=known.Module;
    if(owner==null)continue;
    string hash=Paths.HashFile(file),version="unknown";try{var v=FileVersionInfo.GetVersionInfo(file).FileVersion;if(v!=null){var parsed=new Version(v);version=parsed.Major+"."+parsed.Minor+"."+Math.Max(0,parsed.Build);}}catch(ArgumentException){}
    bool trusted=Catalog.KnownPlugins.Concat(Catalog.Programs).Any(x=>x.Module==owner&&x.Sha256==hash)||result.Receipt.Modules.Any(x=>x.Module==owner&&x.Sha256==hash&&x.Version==version);result.Plugins.Add(new Plugin{Path=relative,Module=owner,Sha256=hash,Version=version,Known=trusted});if(owner=="Core")result.LegacyHost=true;
   }}
   foreach(var module in Catalog.Modules)result.LegacyEnabled[module.Id]=true;
   if(result.LegacyHost){string settings=Paths.Resolve(root,"Plugins/DynastyCOT/settings.json");if(File.Exists(settings)){
    var doc=Json.Read<Dictionary<string,object>>(settings);object format,raw;if(!doc.TryGetValue("format",out format)||!(format is int)||(int)format!=1||!doc.TryGetValue("modules",out raw)||!(raw is Dictionary<string,object>))throw new IOException("Older COT module choices are unreadable; keep them unchanged and repair them first.");
    foreach(var entry in (Dictionary<string,object>)raw){var m=Catalog.Modules.SingleOrDefault(x=>x.LegacyId==entry.Key);if(m==null||!(entry.Value is bool))throw new IOException("Older COT module choices are invalid.");result.LegacyEnabled[m.Id]=(bool)entry.Value;}
   }}return result;
  }
  static string Key(Module m){return "Plugins/"+m.FileName;}
  void Add(Plan plan,string path,byte[] content,string owner,bool deferred=false){
   if(plan.Operations.Any(x=>x.Path.Equals(path,StringComparison.OrdinalIgnoreCase)))throw new IOException("Duplicate operation.");string file=Paths.Resolve(plan.Root,path),before=Paths.Existing(file),after=content==null?null:Paths.Hash(content);if(!deferred&&before==after)return;
   plan.Operations.Add(new Operation{Path=path,Before=before,After=after,Module=owner,Deferred=deferred});if(content!=null)plan.Content[path]=content;
  }
  internal Plan Install(string value,Dictionary<string,ModuleRelease> selected,IEnumerable<PreparedModule> payloads,ISet<string> approvedDowngrades=null){
   string root=Paths.ValidateRoot(value);var inventory=Inspect(root);if(selected.Count==0)throw new IOException("Choose at least one module.");foreach(var entry in selected)ReleaseRules.Validate(Catalog.Get(entry.Key),entry.Value);
   if(inventory.LegacyHost){var missing=Catalog.Modules.Where(m=>inventory.LegacyEnabled[m.Id]&&!selected.ContainsKey(m.Id)).Select(m=>m.Title).ToArray();if(missing.Length>0)throw new IOException("The older COT host still supplies "+String.Join(", ",missing)+". Select those modules to preserve them during this one-time migration, or explicitly remove them first.");}
   var plan=new Plan{Root=root,Action="Install / update",Inventory=inventory};var receipt=inventory.Receipt;
   var prepared=payloads.ToDictionary(x=>x.Module.Id);
   foreach(var entry in selected){var m=Catalog.Get(entry.Key);var target=entry.Value;var current=inventory.Plugins.SingleOrDefault(x=>x.Path.Equals(Key(m),StringComparison.OrdinalIgnoreCase));
    if(current!=null&&current.Version!="unknown"&&ReleaseRules.Numeric(current.Version)>ReleaseRules.Numeric(target.Version)&&(approvedDowngrades==null||!approvedDowngrades.Contains(m.Id)))throw new IOException("Downgrading "+m.Title+" requires choosing and confirming that older version.");
    if(current==null||current.Sha256!=target.DllSha256){PreparedModule p;if(!prepared.TryGetValue(m.Id,out p)||p.Release.Version!=target.Version||p.Release.ArchiveSha256!=target.ArchiveSha256||Paths.HashFile(p.Dll)!=target.DllSha256)throw new IOException("The selected "+m.Title+" payload has not been verified.");
     Add(plan,Key(m),File.ReadAllBytes(p.Dll),m.Id);Add(plan,"COT-Collection/Modules/"+m.Id+"/README.txt",p.Readme,m.Id);Add(plan,"COT-Collection/Modules/"+m.Id+"/FEATURES.md",p.Features,m.Id);plan.Programs.AddRange(p.Programs);
    }
    foreach(var old in inventory.Plugins.Where(x=>x.Module==m.Id&&!x.Path.Equals(Key(m),StringComparison.OrdinalIgnoreCase)))Add(plan,old.Path,null,m.Id);
    receipt.Modules.RemoveAll(x=>x.Module==m.Id);receipt.Modules.Add(new Installed{Module=m.Id,Version=target.Version,Sha256=target.DllSha256});
   }
   if(inventory.LegacyHost){foreach(var host in inventory.Plugins.Where(x=>x.Module=="Core"))Add(plan,host.Path,null,"Core");SnapshotLegacyPrograms(plan);}
   foreach(var m in Catalog.Modules.Where(m=>!selected.ContainsKey(m.Id)))if(inventory.Plugins.Count(x=>x.Module==m.Id)>1)throw new IOException("Duplicate copies of "+m.Title+" remain. Select it to resolve them before continuing.");
   Add(plan,Paths.State,Json.Bytes(receipt),"Core");Finish(plan);return plan;
  }
  internal Plan Remove(string value,IEnumerable<string> selected){
   string root=Paths.ValidateRoot(value);var ids=new HashSet<string>(selected);if(ids.Count==0)throw new IOException("Select modules to remove.");foreach(string id in ids)Catalog.Get(id);
   var inv=Inspect(root);var plan=new Plan{Root=root,Action="Remove selected modules",Inventory=inv};foreach(var plugin in inv.Plugins.Where(x=>ids.Contains(x.Module)))Add(plan,plugin.Path,null,plugin.Module);
   foreach(string id in ids){inv.Receipt.Modules.RemoveAll(x=>x.Module==id);foreach(string name in new[]{"README.txt","FEATURES.md"})Add(plan,"COT-Collection/Modules/"+id+"/"+name,null,id);}
   if(inv.LegacyHost){var choices=new Dictionary<string,bool>();foreach(var m in Catalog.Modules)choices[m.LegacyId]=inv.LegacyEnabled[m.Id]&&!ids.Contains(m.Id);
    if(choices.Values.Any(x=>x))Add(plan,"Plugins/DynastyCOT/settings.json",Json.Bytes(new{format=1,modules=choices}),"Core");else foreach(var host in inv.Plugins.Where(x=>x.Module=="Core"))Add(plan,host.Path,null,"Core");
   }
   Add(plan,Paths.State,Json.Bytes(inv.Receipt),"Core");Finish(plan);return plan;
  }
  void SnapshotLegacyPrograms(Plan plan){
   // Old COT depends on external program files. Save only known code/resources,
   // never settings, identities, histories, saves or arbitrary receipt paths.
   string receipt=Paths.Resolve(plan.Root,"Plugins/DynastyCOT/installed-files.json");if(!File.Exists(receipt))throw new IOException("The old COT host has no installed-files receipt. Restore/repair its receipt before migration so rollback can preserve its matching program files.");
   var doc=Json.Read<Dictionary<string,object>>(receipt);object format,raw;if(!doc.TryGetValue("format",out format)||!(format is int)||(int)format!=1||!doc.TryGetValue("files",out raw)||Json.Array(raw)==null)throw new IOException("Invalid older COT receipt.");
   foreach(var item in Json.Array(raw)){var row=item as Dictionary<string,object>;if(row==null||!row.ContainsKey("path")||!(row["path"] is string)||!row.ContainsKey("sha256")||!(row["sha256"] is string))throw new IOException("Invalid old COT file record.");string path=((string)row["path"]).Replace('\\','/'),hash=(string)row["sha256"];Paths.Relative(path);ReleaseRules.Hash(hash);
    if(!ProgramPath(path))continue;string file=Paths.Resolve(plan.Root,path);if(!File.Exists(file)){
     // Released COT receipts also describe lazily extracted GDA resources.
     // A known host can recreate its own matching native/diagnostic cache;
     // preserve the original absence instead of treating it as damage.
     bool embedded=System.Text.RegularExpressions.Regex.IsMatch(path,@"^DGA-Runtime/Runtime-\d+[.]\d+[.]\d+/(?:DgaDiagnostics[.]exe|(?:DynamicGamedayAtmosphere|GamedayAtmosphere)Native-\d+[.]\d+[.]\d+[.]dll)$")&&Catalog.Programs.Any(x=>x.Module=="GamedayAtmosphere"&&x.Path==path&&x.Sha256==hash)&&plan.Inventory.Plugins.Any(x=>x.Module=="Core"&&x.Known);
     if(!embedded)throw new IOException("Old COT program file is missing: "+path+". Repair the old install before migration.");Add(plan,path,null,"Runtime",true);continue;
    }
    string actual=Paths.HashFile(file);if(!Catalog.Programs.Any(x=>x.Path.Equals(path,StringComparison.OrdinalIgnoreCase)&&x.Sha256==actual))throw new IOException("Old COT program file is unrecognized: "+path+". No program files were changed.");Add(plan,path,null,"Runtime",true);
   }
   foreach(var module in Catalog.Modules.Where(x=>plan.Inventory.LegacyEnabled[x.Id]&&x.Id!="GamedayAtmosphere")){
    bool covered=plan.Operations.Any(x=>x.Deferred&&(module.Id=="BeyondTheField"?x.Path.StartsWith("COT-Runtime/DynastyEnhancer/")&&x.Path.EndsWith(".dll"):module.Id=="ExtraSettings"?x.Path.StartsWith("Plugins/OfflineDynastySettings/")&&x.Path.EndsWith(".exe"):x.Path.StartsWith("Plugins/SituationalPractice/")&&x.Path.EndsWith(".exe")));
    if(!covered)throw new IOException("Old COT receipt is missing the matching program files for "+module.Title+". Repair that receipt before migration so rollback remains complete.");
   }
  }
  internal static bool ProgramPath(string path){return path.StartsWith("COT-Runtime/",StringComparison.Ordinal)&&((path.EndsWith(".dll")||path.EndsWith(".exe"))||path.StartsWith("COT-Runtime/DynastyEnhancer/Tools/")&&path.EndsWith(".mjs")||path.StartsWith("COT-Runtime/DynastyEnhancer/Content/Runtime/")&&path.EndsWith(".json"))||path.StartsWith("DGA-Runtime/Runtime-")&&(path.EndsWith(".dll")||path.EndsWith(".exe"))||System.Text.RegularExpressions.Regex.IsMatch(path,@"^LICENSES/[^/]+[.]txt$")||new[]{"Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe","Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe","Plugins/SituationalPractice/SituationalPractice.Runtime.exe","Plugins/SituationalPractice/ExpandedPractice.Runtime.exe"}.Contains(path);}
  void Finish(Plan plan){plan.Summary=String.Join(Environment.NewLine,plan.Operations.Select(x=>(x.Deferred?"Back up for old-COT rollback: ":x.After==null?"Remove duplicate/plugin: ":x.Before==null?"Install: ":"Update: ")+x.Path));}
  internal List<Plugin> Review(Plan plan){return plan.Inventory.Plugins.Where(x=>!x.Known&&plan.Operations.Any(o=>o.Path.Equals(x.Path,StringComparison.OrdinalIgnoreCase))).ToList();}
  bool Allowed(Operation op){
   Paths.Relative(op.Path);if(op.Module=="Runtime")return op.Deferred&&ProgramPath(op.Path)&&Catalog.Programs.Any(x=>x.Path.Equals(op.Path,StringComparison.OrdinalIgnoreCase))&&op.After==null;
   if(op.Path==Paths.State||op.Path=="Plugins/DynastyCOT/settings.json")return op.Module=="Core";
   if(op.Path.StartsWith("COT-Collection/Modules/")){return Catalog.Modules.Any(m=>m.Id==op.Module&&new[]{"COT-Collection/Modules/"+m.Id+"/README.txt","COT-Collection/Modules/"+m.Id+"/FEATURES.md"}.Contains(op.Path));}
   return op.Path.StartsWith("Plugins/",StringComparison.Ordinal)&&op.Path.EndsWith(".dll",StringComparison.OrdinalIgnoreCase)&&(op.Module=="Core"||Catalog.Modules.Any(m=>m.Id==op.Module));
  }
  void Save(string directory,Journal journal){Paths.Atomic(Path.Combine(directory,"transaction.json"),Json.Bytes(journal));}
  static Mutex OperationMutex(string root){return new Mutex(false,"Local\\Ace.DynastyCOT.Collection."+Paths.Hash(Encoding.UTF8.GetBytes(root.ToUpperInvariant())));}
  static void Acquire(Mutex mutex){try{if(!mutex.WaitOne(0))throw new IOException("Another collection operation is running for this folder.");}catch(AbandonedMutexException){/* The journal, not mutex ownership, controls interrupted-operation recovery. */}}
  internal string Execute(Plan plan,Dictionary<string,string> reviewed,Action<string> progress,int faultAfter=-1){
   string root=Paths.ValidateRoot(plan.Root);Paths.Closed(fixture,root);if(fixture==null&&faultAfter!=-1)throw new IOException("Failure injection is fixture-only.");
   using(var mutex=OperationMutex(root)){
    Acquire(mutex);try{
     if(Backups(root).Any(x=>x.Journal.Status=="installing"||x.Journal.Status=="restore_failed"))throw new IOException("An interrupted operation needs restoration first. Open Backups.");
     foreach(var op in plan.Operations){if(!Allowed(op)||Paths.Existing(Paths.Resolve(root,op.Path))!=op.Before)throw new IOException("An installation file changed after review: "+op.Path);if(op.After!=null&&(!plan.Content.ContainsKey(op.Path)||Paths.Hash(plan.Content[op.Path])!=op.After))throw new IOException("Prepared payload changed.");}
     foreach(var item in Review(plan)){string expected;if(reviewed==null||!reviewed.TryGetValue(item.Path,out expected)||expected!=item.Sha256)throw new IOException("Review and approve backing up this unrecognized plugin: "+item.Path+" ("+item.Sha256+").");}
     Paths.Writable(root,plan.Operations.Select(x=>x.Path).Concat(new[]{Paths.Backups+"/probe",Paths.Log}));
     string directory=Paths.Resolve(root,Paths.Backups+"/"+DateTime.UtcNow.ToString("yyyyMMddTHHmmssfff")+"-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(directory);
     var journal=new Journal{Root=root,Status="preparing",Action=plan.Action,Created=DateTime.Now.ToString("yyyy-MM-dd HH:mm"),Programs=plan.Programs};
     foreach(var op in plan.Operations){op.Backup=journal.Operations.Count.ToString("D4")+".bak";if(op.Before!=null){File.Copy(Paths.Resolve(root,op.Path),Path.Combine(directory,op.Backup));if(Paths.HashFile(Path.Combine(directory,op.Backup))!=op.Before)throw new IOException("Backup verification failed.");}journal.Operations.Add(op);}Save(directory,journal);journal.Status="installing";Save(directory,journal);
     try{int count=0;foreach(var op in journal.Operations){if(op.Deferred)continue;progress((op.After==null?"Removing ":"Installing ")+op.Path);if(op.After==null){if(File.Exists(Paths.Resolve(root,op.Path)))File.Delete(Paths.Resolve(root,op.Path));}else Paths.Atomic(Paths.Resolve(root,op.Path),plan.Content[op.Path]);if(Paths.Existing(Paths.Resolve(root,op.Path))!=op.After)throw new IOException("Installed file verification failed.");if(++count==faultAfter)throw new IOException("Injected transaction failure.");}
      journal.Status="committed";Save(directory,journal);AppendLog(root,plan.Action+" completed; backup "+Path.GetFileName(directory));return directory;
     }catch(Exception failure){try{RestoreCore(directory,journal,true);throw new IOException("Operation failed; previous managed files were restored. "+failure.Message,failure);}catch(IOException restore){if(journal.Status=="rolled_back")throw;journal.Status="restore_failed";Save(directory,journal);throw new IOException("Operation failed and automatic restoration needs attention. Keep backup "+directory+". "+restore.Message,failure);}}
    }finally{mutex.ReleaseMutex();}
   }
  }
  void AppendLog(string root,string text){try{string path=Paths.Resolve(root,Paths.Log);Directory.CreateDirectory(Path.GetDirectoryName(path));File.AppendAllText(path,DateTime.UtcNow.ToString("o")+" "+text+Environment.NewLine);}catch(IOException){}catch(UnauthorizedAccessException){}}
  internal List<Backup> Backups(string root){var result=new List<Backup>();string folder=Paths.Resolve(root,Paths.Backups);if(!Directory.Exists(folder))return result;foreach(string directory in Directory.GetDirectories(folder))try{Paths.NoLinks(root,directory);var journal=Json.Read<Journal>(Path.Combine(directory,"transaction.json"));if(journal.Format==1&&String.Equals(journal.Root,root,StringComparison.OrdinalIgnoreCase)&&new[]{"committed","installing","restore_failed"}.Contains(journal.Status)&&journal.Operations.Count>0)result.Add(new Backup{Directory=directory,Journal=journal});}catch(IOException){}catch(ArgumentException){}catch(InvalidOperationException){}return result.OrderByDescending(x=>x.Directory,StringComparer.Ordinal).ToList();}
  internal void Restore(string root,string directory){root=Paths.ValidateRoot(root);Paths.Closed(fixture,root);using(var mutex=OperationMutex(root)){Acquire(mutex);try{string prefix=Paths.Resolve(root,Paths.Backups)+"\\";directory=Path.GetFullPath(directory);if(!directory.StartsWith(prefix,StringComparison.OrdinalIgnoreCase)||!String.Equals(Path.GetDirectoryName(directory),prefix.TrimEnd('\\'),StringComparison.OrdinalIgnoreCase))throw new IOException("Choose a backup from this Mod Manager.");Paths.NoLinks(root,directory);var journal=Json.Read<Journal>(Path.Combine(directory,"transaction.json"));if(!String.Equals(journal.Root,root,StringComparison.OrdinalIgnoreCase))throw new IOException("Backup belongs to another Mod Manager.");RestoreCore(directory,journal,false);AppendLog(root,"Restored "+Path.GetFileName(directory));}finally{mutex.ReleaseMutex();}}}
  List<KnownFile> RecordedPrograms(string root){
   var result=new List<KnownFile>(Catalog.Programs);string folder=Paths.Resolve(root,Paths.Backups);if(!Directory.Exists(folder))return result;
   foreach(string directory in Directory.GetDirectories(folder).OrderByDescending(x=>x,StringComparer.Ordinal).Take(1000))try{Paths.NoLinks(root,directory);string path=Path.Combine(directory,"transaction.json");if(!File.Exists(path))continue;Paths.NoLinks(root,path);var history=Json.Read<Journal>(path);
    if(history.Format!=1||!String.Equals(history.Root,root,StringComparison.OrdinalIgnoreCase)||history.Programs==null||history.Programs.Count>2000)continue;
    foreach(var file in history.Programs){Paths.Relative(file.Path);ReleaseRules.Hash(file.Sha256);if(Catalog.Modules.Any(x=>x.Id==file.Module)&&Catalog.Programs.Any(x=>x.Path.Equals(file.Path,StringComparison.OrdinalIgnoreCase)))result.Add(file);}
   }catch(IOException){}catch(ArgumentException){}catch(InvalidOperationException){}return result;
  }
  void RestoreCore(string directory,Journal journal,bool automatic){
   if(journal.Format!=1||journal.Root==null||journal.Status=="rolled_back"||journal.Operations==null||journal.Operations.Count>10000||journal.Operations.Select(x=>x.Path).Distinct(StringComparer.OrdinalIgnoreCase).Count()!=journal.Operations.Count)throw new IOException("Invalid backup journal.");string root=Paths.ValidateRoot(journal.Root);if(!directory.StartsWith(Paths.Resolve(root,Paths.Backups)+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("Backup belongs to a different Mod Manager.");
   var verifiedPrograms=RecordedPrograms(root);
   foreach(var op in journal.Operations){if(!Allowed(op)||op.Backup==null||!System.Text.RegularExpressions.Regex.IsMatch(op.Backup,@"^\d{4}[.]bak$"))throw new IOException("Unsafe backup operation.");if(op.Before!=null){ReleaseRules.Hash(op.Before);Paths.NoLinks(root,Path.Combine(directory,op.Backup));if(Paths.HashFile(Path.Combine(directory,op.Backup))!=op.Before)throw new IOException("Backup hash differs.");}if(op.After!=null)ReleaseRules.Hash(op.After);
    string current=Paths.Existing(Paths.Resolve(root,op.Path));if(current!=op.Before&&current!=op.After&&!(op.Deferred&&verifiedPrograms.Any(x=>x.Path.Equals(op.Path,StringComparison.OrdinalIgnoreCase)&&x.Sha256==current)))throw new IOException("File changed since this backup: "+op.Path+". Restore newer backups first; no files were restored.");
   }
   foreach(var op in journal.Operations.AsEnumerable().Reverse()){string file=Paths.Resolve(root,op.Path);if(Paths.Existing(file)==op.Before)continue;if(op.Before==null){if(File.Exists(file))File.Delete(file);}else Paths.Atomic(file,File.ReadAllBytes(Path.Combine(directory,op.Backup)));if(Paths.Existing(file)!=op.Before)throw new IOException("Restore verification failed.");}
   journal.Status="rolled_back";Save(directory,journal);
  }
 }
}
