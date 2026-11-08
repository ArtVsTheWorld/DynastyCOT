using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Web.Script.Serialization;

namespace DynastyCOT.Setup {
 internal sealed class Fixture {
  internal readonly string Root, Parent;
  internal Fixture(string root,string parent) {
   Root=Path.GetFullPath(root).TrimEnd('\\');Parent=Path.GetFullPath(parent).TrimEnd('\\');
   if(!Root.StartsWith(Parent+"\\",StringComparison.OrdinalIgnoreCase)||!File.Exists(Path.Combine(Root,".cot-fixture")))
    throw new IOException("The setup test must use an isolated fixture.");
  }
 }
 internal sealed class Backup {
  internal string Path, Label;
  public override string ToString(){return Label;}
 }
 internal sealed class InstallResult {
  internal bool Success;internal string Details;
 }
 internal sealed class InstallerEngine : IDisposable {
  internal static readonly string[] ModuleIds={"DynastyEnhancer","GamedayAtmosphere","ExtraDynastySettings","DrivePractice"};
  private string extracted;
  private readonly List<string> ownedExtractions=new List<string>();
  private readonly Dictionary<string,string> updateExtractions=new Dictionary<string,string>();
  internal static string InstalledVersion(string root){
   string receipt=Path.Combine(root,"Plugins","DynastyCOT","installed-files.json");
   if(!File.Exists(receipt))return File.Exists(Path.Combine(root,"Plugins","DynastyCOT.dll"))?"Installed (older receipt unavailable)":"Not installed";
   var doc=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(receipt)) as Dictionary<string,object>;object value;
   return doc!=null&&doc.TryGetValue("version",out value)&&value is string?(string)value:"Installed (version unavailable)";
  }
  internal static string Hash(Stream input){using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(input)).Replace("-","").ToLowerInvariant();}
  internal static string Resolve(string root,string relative){
   if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.Contains(":"))throw new IOException("Invalid package path.");
   foreach(string part in relative.Replace('\\','/').Split('/'))if(part=="..")throw new IOException("Invalid package path.");
   string prefix=Path.GetFullPath(root).TrimEnd('\\')+"\\",path=Path.GetFullPath(Path.Combine(prefix,relative.Replace('/','\\')));
   if(!path.StartsWith(prefix,StringComparison.OrdinalIgnoreCase))throw new IOException("Package path leaves its folder.");
   return path;
  }
  internal static string ValidateRoot(string value){
   string path=Path.GetFullPath(value.Trim()).TrimEnd('\\');
   // A parent MMC folder can contain a launcher too. Prefer the actual
   // versioned Mod Manager installation, and never guess between versions.
   var candidates=ManagerChildren(path);
   if(candidates.Count>1)throw new IOException("Several MMC Mod Manager versions found. Browse to the version you use.");
   if(candidates.Count==1)return candidates[0];
   if(!File.Exists(Path.Combine(path,"MMCModManager.exe")))throw new IOException("Choose your MMC Mod Manager folder, usually MMC_ModManager_vX.X.X.X.");
   return path;
  }
  private static List<string> ManagerChildren(string path){
   var result=new List<string>();if(!Directory.Exists(path))return result;
   foreach(string child in Directory.GetDirectories(path,"MMC_ModManager*"))
    if(File.Exists(Path.Combine(child,"MMCModManager.exe")))result.Add(child);
   return result;
  }
  internal static string FindMmc(string start){
   for(var directory=new DirectoryInfo(start);directory!=null;directory=directory.Parent){
    try{
     var candidates=ManagerChildren(directory.FullName);
     if(candidates.Count>1)return null;
     if(candidates.Count==1)return candidates[0];
     if(directory.Name.StartsWith("MMC_ModManager",StringComparison.OrdinalIgnoreCase)&&File.Exists(Path.Combine(directory.FullName,"MMCModManager.exe")))return directory.FullName;
    }catch(UnauthorizedAccessException){}catch(IOException){}
   }return null;
  }
  internal static Dictionary<string,bool> LoadModules(string root){
   var choices=new Dictionary<string,bool>();foreach(string id in ModuleIds)choices.Add(id,true);
   string path=Path.Combine(root,"Plugins","DynastyCOT","settings.json");if(!File.Exists(path))return choices;
   var doc=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(path)) as Dictionary<string,object>;object v;
   if(doc==null||!doc.TryGetValue("format",out v)||!(v is int)||(int)v!=1)throw new IOException("Existing COT module settings could not be read. Details are in settings.json.");
   if(!doc.TryGetValue("modules",out v)||!(v is Dictionary<string,object>))throw new IOException("Existing COT module settings are invalid.");
   foreach(var pair in (Dictionary<string,object>)v){if(!choices.ContainsKey(pair.Key)||!(pair.Value is bool))throw new IOException("Existing COT module settings are invalid.");choices[pair.Key]=(bool)pair.Value;}
   return choices;
  }
  internal static List<Backup> Backups(string root){
   var result=new List<Backup>();string folder=Path.Combine(root,"Plugins","DynastyCOT","Backups");
   if(!Directory.Exists(folder))return result;
   string[] directories=Directory.GetDirectories(folder);Array.Sort(directories,StringComparer.Ordinal);Array.Reverse(directories);
   foreach(string directory in directories)try{
    var doc=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(Path.Combine(directory,"transaction.json"))) as Dictionary<string,object>;
    if(doc==null||!doc.ContainsKey("status")||(string)doc["status"]!="committed"||!doc.ContainsKey("root")||
       !String.Equals(Path.GetFullPath((string)doc["root"]).TrimEnd('\\'),Path.GetFullPath(root).TrimEnd('\\'),StringComparison.OrdinalIgnoreCase))continue;
    var operations=doc["operations"] as object[];if(operations==null||operations.Length==0)continue;
    result.Add(new Backup{Path=directory,Label=Directory.GetCreationTime(directory).ToString("MMM d, yyyy  h:mm tt")+"  -  "+operations.Length+" backed-up changes"});
   }catch(IOException){}catch(ArgumentException){}catch(InvalidCastException){}catch(KeyNotFoundException){}catch(InvalidOperationException){}catch(UnauthorizedAccessException){}
   return result;
  }
  internal string Extract(Action<string> progress){
   if(extracted!=null)return extracted;
   progress("Checking the bundled package...");
   using(var input=Assembly.GetExecutingAssembly().GetManifestResourceStream("DynastyCOT.Package.zip")){
    if(input==null||Hash(input)!=SetupInfo.PayloadSha256)throw new IOException("The setup download is incomplete or has changed. Download a fresh copy.");
    input.Position=0;
    string folder=Path.Combine(Path.GetTempPath(),"DynastyCOT-Setup-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(folder);extracted=folder;ownedExtractions.Add(folder);
    using(var archive=new ZipArchive(input,ZipArchiveMode.Read)){
     var seen=new HashSet<string>(StringComparer.OrdinalIgnoreCase);
     foreach(var entry in archive.Entries){
      string path=Resolve(folder,entry.FullName);if(!seen.Add(path))throw new IOException("Duplicate package entry.");
      if(entry.FullName.EndsWith("/")){Directory.CreateDirectory(path);continue;}
      Directory.CreateDirectory(Path.GetDirectoryName(path));using(var source=entry.Open())using(var dest=new FileStream(path,FileMode.CreateNew,FileAccess.Write))source.CopyTo(dest);
     }
    }
    return folder;
   }
  }
  internal string ExtractUpdate(VerifiedUpdate update,Action<string> progress){
   if(update==null||update.Metadata==null)throw new IOException("No verified update is selected.");
   using(var input=File.OpenRead(update.Archive))if(input.Length!=update.Metadata.ArchiveBytes||Hash(input)!=update.Metadata.ArchiveSha256)throw new IOException("The downloaded package changed after verification. Download it again.");
   string folder;
   if(!updateExtractions.TryGetValue(update.Metadata.ArchiveSha256,out folder)){
    progress("Checking the selected release package...");folder=Path.Combine(Path.GetTempPath(),"DynastyCOT-Setup-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(folder);ownedExtractions.Add(folder);
    using(var input=File.OpenRead(update.Archive))using(var archive=new ZipArchive(input,ZipArchiveMode.Read)){
     var seen=new HashSet<string>(StringComparer.OrdinalIgnoreCase);long unpacked=0;
     foreach(var entry in archive.Entries){
      string path=Resolve(folder,entry.FullName);if(!seen.Add(path))throw new IOException("Duplicate package entry.");
      unpacked+=entry.Length;if(unpacked>2L*1024*1024*1024)throw new IOException("Selected package exceeds its extraction limit.");
      if(entry.FullName.EndsWith("/")){Directory.CreateDirectory(path);continue;}
      Directory.CreateDirectory(Path.GetDirectoryName(path));using(var source=entry.Open())using(var dest=new FileStream(path,FileMode.CreateNew,FileAccess.Write))source.CopyTo(dest);
     }
    }updateExtractions.Add(update.Metadata.ArchiveSha256,folder);
   }
   var manifest=new JavaScriptSerializer{MaxJsonLength=4*1024*1024}.DeserializeObject(File.ReadAllText(Path.Combine(folder,"package-manifest.json"))) as Dictionary<string,object>;
   if(manifest==null||UpdateClient.StringField(manifest,"product",100)!="DynastyCOT"||UpdateClient.StringField(manifest,"version",100)!=update.Metadata.Version)throw new IOException("Selected package manifest does not match the release.");
   object entries;if(!manifest.TryGetValue("files",out entries)||!(entries is object[]))throw new IOException("Selected package manifest has no file list.");
   var paths=new HashSet<string>(StringComparer.OrdinalIgnoreCase);
   foreach(object item in (object[])entries){
    var entry=item as Dictionary<string,object>;string relative=UpdateClient.StringField(entry,"path",2048),expected=UpdateClient.StringField(entry,"sha256",64),path=Resolve(folder,relative);
    if(!paths.Add(path)||!File.Exists(path))throw new IOException("Selected package file is missing or duplicated.");
    using(var input=File.OpenRead(path))if(Hash(input)!=expected)throw new IOException("Selected package file changed: "+relative);
   }
   if(!paths.Contains(Path.Combine(folder,"Install.ps1")))throw new IOException("Selected package has no verified installer.");
   foreach(string file in Directory.GetFiles(folder,"*",SearchOption.AllDirectories))if(!String.Equals(file,Path.Combine(folder,"package-manifest.json"),StringComparison.OrdinalIgnoreCase)&&!paths.Contains(file))throw new IOException("Selected package contains an unlisted file.");return folder;
  }
  internal static string Quote(string value){return "'"+value.Replace("'","''")+"'";}
  internal InstallResult Run(string root,Dictionary<string,bool> modules,string rollback,Fixture fixture,Action<string> progress,string action="Install",VerifiedUpdate update=null){
   root=ValidateRoot(root);
   if(fixture!=null&&!String.Equals(root,fixture.Root,StringComparison.OrdinalIgnoreCase))throw new IOException("The test target changed.");
   string folder=update!=null&&rollback==null&&action=="Install"?ExtractUpdate(update,progress):Extract(progress),script=Path.Combine(folder,"Install.ps1");
   string call="& "+Quote(script)+" -MmcRoot "+Quote(root)+" -PackageRoot "+Quote(folder);
   if(fixture!=null)call+=" -Fixture -FixtureRoot "+Quote(fixture.Parent);
   if(rollback!=null){
    string expected=Path.Combine(root,"Plugins","DynastyCOT","Backups")+"\\";
    if(!Path.GetFullPath(rollback).StartsWith(expected,StringComparison.OrdinalIgnoreCase))throw new IOException("Choose a COT backup from this MMC folder.");
    call+=" -Rollback "+Quote(rollback);
   }else{
    if(action!="Install"&&action!="UninstallSelected"&&action!="UninstallAll")throw new IOException("Invalid setup action.");
    if(modules==null||modules.Count!=4)throw new IOException("Choose your COT modules.");
    foreach(string id in ModuleIds)if(!modules.ContainsKey(id))throw new IOException("Missing COT module choice.");
    call+=" -ModuleChoicesJson "+Quote(new JavaScriptSerializer().Serialize(modules))+" -Action "+Quote(action);
   }
   string command="$ErrorActionPreference='Stop';$ProgressPreference='SilentlyContinue';try { "+call+";exit 0 } catch { [Console]::Error.WriteLine($_.Exception.Message);exit 1 }";
   var start=new ProcessStartInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System),"WindowsPowerShell","v1.0","powershell.exe"),
    "-NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -EncodedCommand "+Convert.ToBase64String(Encoding.Unicode.GetBytes(command))){
     UseShellExecute=false,CreateNoWindow=true,WindowStyle=ProcessWindowStyle.Hidden,RedirectStandardOutput=true,RedirectStandardError=true,WorkingDirectory=folder};
   var details=new StringBuilder();object sync=new object();
   // Windows PowerShell must discover its own modules even when setup is
   // launched by a development shell that exports PowerShell 7's module path.
   start.EnvironmentVariables.Remove("PSModulePath");
   DataReceivedEventHandler receive=(s,e)=>{if(e.Data!=null){lock(sync)details.AppendLine(e.Data);progress(e.Data);}};
   progress(rollback!=null?"Restoring the selected backup...":action=="Install"?"Installing COT and creating a backup...":"Removing COT files and creating a backup...");
   using(var process=new Process{StartInfo=start}){
    process.OutputDataReceived+=receive;process.ErrorDataReceived+=receive;process.Start();process.BeginOutputReadLine();process.BeginErrorReadLine();process.WaitForExit();
    return new InstallResult{Success=process.ExitCode==0,Details=details.ToString()};
   }
  }
  public void Dispose(){
   // Only remove this instance's uniquely created extraction directory.
   foreach(string owned in ownedExtractions)try{string path=Path.GetFullPath(owned),parent=Path.GetFullPath(Path.GetTempPath()).TrimEnd('\\')+"\\";
    if(path.StartsWith(parent+"DynastyCOT-Setup-",StringComparison.OrdinalIgnoreCase)&&Path.GetDirectoryName(path)+"\\"==parent)Directory.Delete(path,true);
   }catch(IOException){}catch(UnauthorizedAccessException){}
  }
 }
}
