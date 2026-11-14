using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;

namespace DynastyCOT.Collection {
 internal static class Json {
  internal static JavaScriptSerializer Serializer(){return new JavaScriptSerializer{MaxJsonLength=8*1024*1024,RecursionLimit=48};}
  internal static T Read<T>(string file){if(new FileInfo(file).Length>8*1024*1024)throw new IOException("JSON file exceeds its limit.");return Serializer().Deserialize<T>(File.ReadAllText(file,Encoding.UTF8));}
  internal static byte[] Bytes(object value){return new UTF8Encoding(false).GetBytes(Serializer().Serialize(value));}
  internal static object[] Array(object value){var list=value as System.Collections.IList;return list==null?null:list.Cast<object>().ToArray();}
 }
 internal static class DisplayText {
  internal static string Notes(string value){return (value??"").Replace("\r\n","\n").Replace("\r","\n").Replace("\n","\r\n");}
 }
 internal sealed class Module {
  public string Id {get;set;} public string LegacyId {get;set;} public string Title {get;set;} public string Description {get;set;} public string Repository {get;set;}
  public string[] Aliases {get;set;}
  public ModuleRelease Current {get;set;}
  public string FileName {get{return Id+".dll";}}
  internal string GuideName {get{return Id+"-Player-Guide.pdf";}}
 }
 internal sealed class ModuleRelease {
  public string Module,Version,Tag,ArchiveName,ArchiveUrl,ArchiveSha256,DllSha256,ProofUrl,PageUrl,Notes,Compatibility;
  public long ArchiveBytes;
  public bool Prerelease;
  public override string ToString(){return Version+(Prerelease?" (test release)":"");}
 }
 internal sealed class KnownFile {public string Path {get;set;} public string Module {get;set;} public string Sha256 {get;set;}}
 internal sealed class Catalog {
  public int Format {get;set;}
  public string Version {get;set;}
  public Module[] Modules {get;set;}
  public KnownFile[] KnownPlugins {get;set;} public KnownFile[] Programs {get;set;}
  internal Module Get(string id){var value=Modules.SingleOrDefault(x=>x.Id==id);if(value==null)throw new IOException("Unknown module: "+id);return value;}
  internal void Validate(){
   if(Format!=1||Modules==null||Modules.Length!=4||Modules.Select(x=>x.Id).Distinct().Count()!=4)throw new IOException("Invalid collection catalog.");
   foreach(var m in Modules){if(!new[]{"BeyondTheField","GamedayAtmosphere","ExtraSettings","ExpandedPractice"}.Contains(m.Id)||m.Current.Module!=m.Id||m.Repository!="ArtVsTheWorld/"+(m.Id=="GamedayAtmosphere"?"DynamicGamedayAtmosphere":m.Id=="ExtraSettings"?"offline-dynasty-settings":m.Id=="ExpandedPractice"?"drive-based-practice":"BeyondTheField"))throw new IOException("Invalid module identity.");ReleaseRules.Validate(m,m.Current);}
   foreach(var file in (KnownPlugins??new KnownFile[0]).Concat(Programs??new KnownFile[0])){Paths.Relative(file.Path);ReleaseRules.Hash(file.Sha256);}
  }
 }
 internal static class ReleaseRules {
  internal static void Hash(string value){if(value==null||!Regex.IsMatch(value,"^[a-f0-9]{64}$"))throw new IOException("Missing or invalid SHA-256.");}
  internal static Version Numeric(string value){if(value==null||!Regex.IsMatch(value,@"^\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?$"))throw new IOException("Invalid release version.");return new Version(value.Split('-')[0]);}
  internal static void Validate(Module module,ModuleRelease release){
   Numeric(release.Version);if(release.Module!=module.Id||release.Tag!="v"+release.Version||release.ArchiveName!=module.Id+"-"+release.Version+".zip"||release.ArchiveBytes<=0||release.ArchiveBytes>256L*1024*1024)throw new IOException("Release identity/size differs from the selected module.");
   Hash(release.ArchiveSha256);if(release.DllSha256!=null)Hash(release.DllSha256);
   NetworkUrl(module,release.ArchiveUrl,false);
   string prefix="https://github.com/"+module.Repository+"/releases/download/"+release.Tag+"/";
   if(release.ArchiveUrl!=prefix+release.ArchiveName||release.ProofUrl!=prefix+module.Id+"-"+release.Version+"-verification.json")throw new IOException("Release asset address differs from its version.");
  }
  internal static Uri NetworkUrl(Module module,string value,bool api){
   Uri u;if(!Uri.TryCreate(value,UriKind.Absolute,out u)||u.Scheme!="https"||!String.IsNullOrEmpty(u.UserInfo)||!u.IsDefaultPort)throw new IOException("Updates require an HTTPS GitHub address.");
   var host=api?"api.github.com":"github.com";var prefix=api?"/repos/"+module.Repository+"/":"/"+module.Repository+"/releases/";
   bool releasePage=api&&u.AbsolutePath=="/repos/"+module.Repository+"/releases"&&Regex.IsMatch(u.Query,@"^\?per_page=100&page=[1-9]\d{0,3}$");
   if(u.Host!=host||!u.AbsolutePath.StartsWith(prefix,StringComparison.Ordinal)||(u.Query.Length!=0&&!releasePage)||u.Fragment.Length!=0)throw new IOException("Release belongs to another repository.");return u;
  }
 }
 internal sealed class Installed {public string Module,Version,Sha256;}
 internal sealed class Receipt {public int Format=1;public List<Installed> Modules=new List<Installed>();}
 internal sealed class Plugin {public string Path,Module,Sha256,Version;public bool Known;}
 internal sealed class Inventory {
  public List<Plugin> Plugins=new List<Plugin>();
  public Dictionary<string,bool> LegacyEnabled=new Dictionary<string,bool>();
  public bool LegacyHost;
  public Receipt Receipt=new Receipt();
  internal string Status(Module module){var list=Plugins.Where(x=>x.Module==module.Id).ToList();if(list.Count>1)return "Duplicate copies found";if(list.Count==1)return "Installed "+list[0].Version+(list[0].Known?"":" (review needed)");return LegacyHost&&LegacyEnabled[module.Id]?"Installed through older COT":"Not installed";}
 }
 internal sealed class PreparedModule {
  internal Dictionary<string,byte[]> SupportFiles=new Dictionary<string,byte[]>(StringComparer.OrdinalIgnoreCase);
  internal Module Module;internal ModuleRelease Release;internal string Dll;
  internal byte[] Readme,Features,Guide;
  internal List<KnownFile> Programs=new List<KnownFile>();
 }
 internal sealed class Operation {
  public string Path,Before,After,Backup,Module;
  public bool Deferred;
 }
 internal sealed class Journal {
  public int Format=1;
  public string Root,Status,Action,Created;
  public List<Operation> Operations=new List<Operation>();
  public List<KnownFile> Programs=new List<KnownFile>();
 }
 internal sealed class Plan {
  internal string Root,Action;
  internal List<Operation> Operations=new List<Operation>();
  internal Dictionary<string,byte[]> Content=new Dictionary<string,byte[]>(StringComparer.OrdinalIgnoreCase);
  internal Inventory Inventory;
  internal List<KnownFile> Programs=new List<KnownFile>();
  internal string Summary;
 }
 internal sealed class Backup {
  internal string Directory;internal Journal Journal;
  public override string ToString(){return Journal.Created+" — "+Journal.Action+(Journal.Status=="installing"?" (interrupted)":"");}
 }
}
