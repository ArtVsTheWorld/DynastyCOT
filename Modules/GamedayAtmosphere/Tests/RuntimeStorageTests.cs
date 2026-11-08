using System;using System.IO;using System.Linq;using DynamicGamedayAtmosphere;
class RuntimeStorageTests {
 static void Check(bool okay){if(!okay)throw new Exception("Runtime-storage regression");}
 static int Main(){
  string root=Path.Combine(Path.GetTempPath(),"DgaRuntimeStorage-"+Guid.NewGuid().ToString("N"));
  try{
   string plugins=Path.Combine(root,"Plugins");Directory.CreateDirectory(plugins);
   string data=PluginStorage.DataFolder(plugins),runtime=PluginStorage.RuntimeFolder(plugins,"2.1.2");
   Check(runtime==Path.Combine(root,"DGA-Runtime","Runtime-2.1.2")&&!runtime.StartsWith(plugins+Path.DirectorySeparatorChar));
   Check(PluginStorage.FindPluginsFolder(Path.Combine(plugins,"DynamicGamedayAtmosphere.dll"))==plugins);
   Check(PluginStorage.FindPluginsFolder(Path.Combine(plugins,"Extra Folder","DynamicGamedayAtmosphere.dll"))==plugins);
   bool refused=false;try{PluginStorage.FindPluginsFolder(Path.Combine(root,"Unrelated","plugin.dll"));}catch(IOException){refused=true;}Check(refused);
   refused=false;try{PluginStorage.RuntimeFolder(plugins,"..\\Escape");}catch(ArgumentException){refused=true;}Check(refused);
   string previous=Path.Combine(data,"Runtime-2.0.35");Directory.CreateDirectory(previous);
   string[] owned={"DynamicGamedayAtmosphereNative-2.0.35.dll","LoadDynamicGamedayAtmosphere.exe"};
   foreach(string name in owned)File.WriteAllText(Path.Combine(previous,name),name);
   File.WriteAllText(Path.Combine(previous,"AnotherPlugin.dll"),"unrelated");
   File.WriteAllText(Path.Combine(previous,"DynamicGamedayAtmosphereNative-custom.dll"),"unrelated");
   Directory.CreateDirectory(Path.Combine(data,"History"));File.WriteAllText(Path.Combine(data,"History","audience-foundations.txt"),"keep foundation");
   File.WriteAllText(PluginStorage.SettingsFile(plugins),"keep settings");
   Check(PluginStorage.NeedsRuntimeRetirement(plugins));
   string locked=Path.Combine(previous,owned[0]);
   using(var held=new FileStream(locked,FileMode.Open,FileAccess.Read,FileShare.Read)){
    refused=false;try{PluginStorage.RetirePreviousRuntimes(plugins);}catch(IOException){refused=true;}Check(refused&&File.ReadAllText(locked)==owned[0]);
   }
   PluginStorage.RetirePreviousRuntimes(plugins);Check(!PluginStorage.NeedsRuntimeRetirement(plugins));
   Check(PluginStorage.RetirePreviousRuntimes(plugins)==0);
   var backups=Directory.GetFiles(Path.Combine(root,"DGA-Runtime","Retired"),"*.bak",SearchOption.AllDirectories);
   Check(backups.Length==2&&owned.All(n=>backups.Any(p=>File.ReadAllText(p)==n)));
   Check(File.ReadAllText(Path.Combine(previous,"AnotherPlugin.dll"))=="unrelated"&&File.ReadAllText(Path.Combine(previous,"DynamicGamedayAtmosphereNative-custom.dll"))=="unrelated");
   Check(File.ReadAllText(PluginStorage.SettingsFile(plugins))=="keep settings"&&File.ReadAllText(Path.Combine(data,"History","audience-foundations.txt"))=="keep foundation");
   Console.WriteLine("PASS: one-DLL paths, nested install lookup, version validation, exact old-runtime retirement, locks/retry, idempotence, backups, settings/history preservation and unrelated-file isolation. No installed files changed.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
  finally{if(Path.GetFullPath(root).StartsWith(Path.Combine(Path.GetTempPath(),"DgaRuntimeStorage-"),StringComparison.OrdinalIgnoreCase)&&Directory.Exists(root))Directory.Delete(root,true);}
 }
}
