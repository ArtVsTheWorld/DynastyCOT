using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace DynamicGamedayAtmosphere {
    // Only DGA-owned names are migrated. Other plugins' settings/logs are untouched.
    public static class PluginStorage {
        public static string FindPluginsFolder(string assemblyFile){
            for(var dir=new DirectoryInfo(Path.GetDirectoryName(Path.GetFullPath(assemblyFile)));dir!=null;dir=dir.Parent)
                if(String.Equals(dir.Name,"Plugins",StringComparison.OrdinalIgnoreCase))return dir.FullName;
            throw new IOException("Install GamedayAtmosphere.dll in MMC's Plugins folder.");
        }
        public static string RuntimeRoot(string plugins){
            var dir=new DirectoryInfo(Path.GetFullPath(plugins));
            if(!String.Equals(dir.Name,"Plugins",StringComparison.OrdinalIgnoreCase)||dir.Parent==null)throw new IOException("Cannot locate MMC's Plugins folder for runtime storage.");
            return Ace.DynastyCOT.UserStorage.Folder("GamedayAtmosphere");
        }
        public static string RuntimeFolder(string plugins,string version){
            if(!Regex.IsMatch(version,@"^\d+\.\d+\.\d+$"))throw new ArgumentException("Invalid runtime version.");
            return Path.Combine(RuntimeRoot(plugins),"Runtime-"+version);
        }
        static IEnumerable<string> PreviousRuntimeFiles(string plugins){
            string data=DataFolder(plugins);if(!Directory.Exists(data)||(File.GetAttributes(data)&FileAttributes.ReparsePoint)!=0)yield break;
            foreach(string dir in Directory.GetDirectories(data,"Runtime-*")){
                if(!Regex.IsMatch(Path.GetFileName(dir),@"^Runtime-\d+\.\d+\.\d+$")||(File.GetAttributes(dir)&FileAttributes.ReparsePoint)!=0)continue;
                foreach(string file in LooseHelpers(dir)){
                    if((File.GetAttributes(file)&FileAttributes.ReparsePoint)==0)yield return file;
                }
            }
        }
        public static bool NeedsRuntimeRetirement(string plugins){foreach(string file in PreviousRuntimeFiles(plugins))return true;return false;}
        // Call only after the game is closed. Exact owned helper names are
        // archived outside MMC's recursive DLL scan; settings/history stay put.
        public static int RetirePreviousRuntimes(string plugins){
            int count=0;foreach(string file in PreviousRuntimeFiles(plugins)){
                string backup=Path.Combine(RuntimeRoot(plugins),"Retired",Path.GetFileName(Path.GetDirectoryName(file)));
                Retire(file,backup);++count;
            }
            return count;
        }
        // Only exact legacy runtime/helper names; never settings or history.
        public static int RetireLooseRuntimeFiles(string plugins){
            int count=0;foreach(string file in LooseHelpers(plugins)){
                if((File.GetAttributes(file)&FileAttributes.ReparsePoint)!=0)continue;
                Retire(file,Path.Combine(RuntimeRoot(plugins),"Retired","Loose"));++count;
            }
            return count;
        }
        public static string DataFolder(string plugins){return Ace.DynastyCOT.UserStorage.Folder("GamedayAtmosphere");}
        public static string SettingsFile(string plugins){return Path.Combine(DataFolder(plugins),"DynamicGamedayAtmosphere.ini");}
        static void Retire(string path,string folder){
            if(!File.Exists(path))return;
            SettingsIo.Run("create retired-file backup folder",null,folder,null,()=>Directory.CreateDirectory(folder));
            string saved=Path.Combine(folder,Path.GetFileName(path)+"-"+Guid.NewGuid().ToString("N")+".bak");
            SettingsIo.Run("archive previous DGA file",path,saved,null,()=>File.Copy(path,saved,false));
        }
        static string ExistingSettings(string plugins){
            foreach(string path in new[]{SettingsFile(plugins),Path.Combine(plugins,"DynamicGamedayAtmosphere","DynamicGamedayAtmosphere.ini"),Path.Combine(plugins,"DynamicGamedayAtmosphere.ini"),Path.Combine(plugins,"DynamicAttendance.ini")})
                if(File.Exists(path))return path;
            return null;
        }
        // Exact DGA helper names/versioned native runtimes only. The MMC adapter
        // stays beside the other plugins; unknown files are never swept up.
        static IEnumerable<string> LooseHelpers(string plugins){
            if(!Directory.Exists(plugins))yield break;
            foreach(string file in Directory.GetFiles(plugins)){
                string name=Path.GetFileName(file);
                if(String.Equals(name,"LoadDynamicGamedayAtmosphere.exe",StringComparison.OrdinalIgnoreCase)||String.Equals(name,"LoadDynamicAttendance.exe",StringComparison.OrdinalIgnoreCase)||String.Equals(name,"DgaDiagnostics.exe",StringComparison.OrdinalIgnoreCase)||
                   Regex.IsMatch(name,@"^(DynamicGamedayAtmosphereNative|GamedayAtmosphereNative|DynamicAttendanceNative)-\d+\.\d+\.\d+\.dll$",RegexOptions.IgnoreCase))yield return file;
            }
        }
        public static bool NeedsRelocation(string plugins){foreach(string file in LooseHelpers(plugins)){if(File.Exists(file))return true;}return false;}
        public static bool NeedsReset(string plugins,string defaults){
            string path=ExistingSettings(plugins);if(path==null)return false;
            string existing=SettingsIo.ReadText(path);
            // A future schema belongs to a newer plugin. Preserve its refusal path.
            if(SettingsMigration.Number(existing,"General","ConfigVersion",1)>SettingsMigration.Number(defaults,"General","ConfigVersion",1))return false;
            int required=SettingsMigration.Number(defaults,"General","SettingsResetVersion",0);
            return required>0&&SettingsMigration.Number(existing,"General","SettingsResetVersion",0)<required;
        }
        // Current distribution paths never reset or retune existing choices.
        // Legacy flat INIs are copied byte-for-byte; originals remain available.
        public static void EnsurePreserving(string plugins,string defaults){Ace.DynastyCOT.UserStorage.Current.WithModule("GamedayAtmosphere",()=>EnsurePreservingCore(plugins,defaults));}
        static void EnsurePreservingCore(string plugins,string defaults){
            Ace.DynastyCOT.UserStorage.Migrate("GamedayAtmosphere",plugins);
            string data=DataFolder(plugins),destination=SettingsFile(plugins);
            SettingsIo.Run("create DGA settings folder",null,data,null,()=>Directory.CreateDirectory(data));
            if(File.Exists(destination))return;
            string source=ExistingSettings(plugins),temp=Path.Combine(data,"settings-"+Guid.NewGuid().ToString("N")+".tmp");Exception failure=null;
            try{
                if(source!=null)SettingsIo.Run("copy existing DGA settings unchanged",source,temp,null,()=>Ace.DynastyCOT.UserStore.Atomic(temp,File.ReadAllBytes(source)));
                else SettingsIo.Run("write initial DGA defaults",null,temp,null,()=>File.WriteAllText(temp,defaults,Encoding.Unicode));
                SettingsIo.Run("activate initial DGA settings",temp,destination,null,()=>File.Move(temp,destination));
            }catch(Exception error){failure=error;throw;}
            finally{SettingsIo.Cleanup(temp,failure);}
        }
        public static string Ensure(string plugins,string defaults){
            string data=DataFolder(plugins),destination=SettingsFile(plugins);
            SettingsIo.Run("create DGA settings folder",null,data,null,()=>Directory.CreateDirectory(data));
            string modern=Path.Combine(plugins,"DynamicGamedayAtmosphere.ini"),legacy=Path.Combine(plugins,"DynamicAttendance.ini");
            if(!File.Exists(destination)){
                string source=File.Exists(modern)?modern:File.Exists(legacy)?legacy:null;
                // Copy before upgrading so even a future-schema file reaches
                // its new location unchanged and can refuse activation safely.
                if(source!=null)SettingsIo.Run("copy previous settings into DGA folder",source,destination,null,()=>File.Copy(source,destination,false));
            }
            string backup=Path.Combine(data,"Backups"),resetBackup=null;
            if(NeedsReset(plugins,defaults)){
                SettingsIo.Run("create settings-reset backup folder",null,backup,null,()=>Directory.CreateDirectory(backup));
                // Short, unique names keep migration within legacy MMC path limits.
                resetBackup=Path.Combine(backup,"settings-before-update3-"+Guid.NewGuid().ToString("N")+".bak");
                string temp=Path.Combine(data,"reset-"+Guid.NewGuid().ToString("N")+".tmp");Exception failure=null;
                try{
                    // Replace atomically and keep the exact old bytes. Failure leaves
                    // the original active and stops startup instead of losing settings.
                    SettingsIo.Run("write temporary Update 3 settings",null,temp,null,()=>File.WriteAllText(temp,defaults,Encoding.Unicode));
                    SettingsIo.Run("replace settings with Update 3 defaults",temp,destination,resetBackup,()=>File.Replace(temp,destination,resetBackup,true));
                }catch(Exception error){failure=error;throw;}
                finally{SettingsIo.Cleanup(temp,failure);}
            }
            SettingsMigration.Ensure(destination,legacy,defaults);
            if(!File.Exists(destination))throw new IOException("DGA settings were not created; old settings have been left in place.");
            Retire(modern,backup);Retire(legacy,backup);
            foreach(string file in new[]{modern,legacy})foreach(string suffix in new[]{".previous",".tmp"})Retire(file+suffix,backup);
            // Do not execute showcase commands left by an older runtime.
            Retire(Path.Combine(plugins,"DynamicGamedayAtmosphere-control.txt"),backup);
            foreach(string name in new[]{"DynamicGamedayAtmosphere.log","DynamicGamedayAtmosphere-launcher.log","DynamicGamedayAtmosphere-status.txt"})
                Retire(Path.Combine(plugins,name),backup);
            foreach(string file in LooseHelpers(plugins))Retire(file,backup);
            return resetBackup;
        }
    }
}
