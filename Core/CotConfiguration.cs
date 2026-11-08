using System;
using System.Collections.Generic;
using System.IO;
using System.Web.Script.Serialization;
namespace DynastyCOT {
 public sealed class Configuration {
  public static readonly string[] ModuleIds={"DynastyEnhancer","GamedayAtmosphere","ExtraDynastySettings","DrivePractice"};
  private readonly Dictionary<string,bool> enabled=new Dictionary<string,bool>(StringComparer.Ordinal);
  public Configuration(){foreach(string id in ModuleIds)enabled.Add(id,true);}
  public bool IsEnabled(string id){bool value;if(!enabled.TryGetValue(id,out value))throw new ArgumentException("Unknown COT module: "+id);return value;}
  public static Configuration Parse(string json){
   var root=new JavaScriptSerializer().DeserializeObject(json) as Dictionary<string,object>;object value;
   if(root==null || !root.TryGetValue("format",out value) || !(value is int) || (int)value!=1)throw new InvalidDataException("Unsupported COT configuration format.");
   Dictionary<string,object> choices;
   if(!root.TryGetValue("modules",out value) || (choices=value as Dictionary<string,object>)==null)throw new InvalidDataException("Missing COT module choices.");
   var result=new Configuration();
   foreach(var item in choices){if(!result.enabled.ContainsKey(item.Key) || !(item.Value is bool))throw new InvalidDataException("Invalid COT module choice: "+item.Key);result.enabled[item.Key]=(bool)item.Value;}
   return result;
  }
  public static Configuration Load(string path){return File.Exists(path)?Parse(File.ReadAllText(path)):new Configuration();}
 }
 public static class ModulePlanner {
  public static Dictionary<string,string> StartSelected(Configuration config,Action<string> verify,Action<string> start){
   var result=new Dictionary<string,string>();
   foreach(string id in Configuration.ModuleIds){
    if(!config.IsEnabled(id)){result[id]="disabled";continue;}
    try{verify(id);start(id);result[id]="startup_requested";}
    catch(Exception error){result[id]="unavailable: "+error.Message;}
   }
   return result;
  }
 }
 public static class CotPaths {
  public static string Plugins {get{return Path.GetDirectoryName(typeof(CotPaths).Assembly.Location);}}
  public static string Mmc {get{return Path.GetDirectoryName(Plugins);}}
  public static string Data {get{return Path.Combine(Plugins,"DynastyCOT");}}
  public static string Runtime(string module){return Path.Combine(Mmc,"COT-Runtime",module);}
  public static string Resolve(string root,string relative){
   if(String.IsNullOrWhiteSpace(relative)||Path.IsPathRooted(relative)||relative.Contains(":"))throw new IOException("Invalid COT payload path.");
   string prefix=Path.GetFullPath(root).TrimEnd(Path.DirectorySeparatorChar)+Path.DirectorySeparatorChar;
   string path=Path.GetFullPath(Path.Combine(prefix,relative.Replace('/',Path.DirectorySeparatorChar)));
   if(!path.StartsWith(prefix,StringComparison.OrdinalIgnoreCase))throw new IOException("COT payload path escapes installation.");
   return path;
  }
 }
}
