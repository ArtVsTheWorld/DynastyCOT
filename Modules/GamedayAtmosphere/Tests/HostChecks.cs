using System;
using System.IO;
using System.Reflection;
using System.Threading;
class HostChecks {
    static string root;
    static int Main(string[] args){
        try {
            root=Path.GetFullPath(args[0]);
            AppDomain.CurrentDomain.AssemblyResolve+=(sender,request)=>{
                string name=new AssemblyName(request.Name).Name+".dll";
                foreach(string dir in new[]{root,Path.Combine(root,"ThirdParty"),Path.Combine(root,"Plugins")}){
                    string p=Path.Combine(dir,name);if(File.Exists(p))return Assembly.LoadFrom(p);
                }
                return null;
            };
            var core=Assembly.LoadFrom(Path.Combine(root,"FrostyCore.dll"));
            string pluginFolder=args.Length>1?Path.GetFullPath(args[1]):Path.Combine(root,"Plugins");
            var plugin=Assembly.LoadFrom(Path.Combine(pluginFolder,"DynamicGamedayAtmosphere.dll"));
            string release=plugin.GetName().Version.ToString(3);
            var action=plugin.GetType("DynamicGamedayAtmosphere.AttendanceAction",true);
            if(!core.GetType("Frosty.Core.ExecutionAction",true).IsAssignableFrom(action))throw new Exception("Execution ABI mismatch");
            var instance=Activator.CreateInstance(action);
            foreach(string name in new[]{"PreLaunchAction","PostLaunchAction"}){
                var callback=(Delegate)action.GetProperty(name).GetValue(instance,null);
                var p=callback.Method.GetParameters();
                if(p.Length!=3||p[0].ParameterType.FullName!="FrostySdk.Interfaces.ILogger"||p[1].ParameterType!=core.GetType("Frosty.Core.PluginManagerType",true)||p[2].ParameterType!=typeof(CancellationToken))throw new Exception("Launch callback ABI mismatch");
            }
            bool registered=false;
            foreach(var attr in plugin.GetCustomAttributesData())if(attr.AttributeType.Name.StartsWith("RegisterExecutionAction")&&(Type)attr.ConstructorArguments[0].Value==action)registered=true;
            if(!registered)throw new Exception("Missing execution action registration");
            foreach(var item in new[]{new[]{"PluginAuthorAttribute","Ace"},new[]{"PluginDisplayNameAttribute","Gameday Atmosphere"}}){
                bool found=false;foreach(var attr in plugin.GetCustomAttributesData())if(attr.AttributeType.Name==item[0]&&Convert.ToString(attr.ConstructorArguments[0].Value)==item[1])found=true;
                if(!found)throw new Exception("Metadata mismatch: "+item[0]);
            }
            var extract=action.GetMethod("Extract",BindingFlags.Static|BindingFlags.NonPublic);
            bool direct=(bool)action.GetField("DirectRuntimeLoading",BindingFlags.Static|BindingFlags.NonPublic).GetRawConstantValue();
            bool external=(bool)action.GetField("ExternalRuntimeLoading",BindingFlags.Static|BindingFlags.NonPublic).GetRawConstantValue();
            if((direct||external)&&plugin.GetManifestResourceStream("Attendance.LoadDynamicGamedayAtmosphere.exe")!=null)throw new Exception("Private variant still embeds the loader EXE");
            string runtime=(string)action.GetProperty("RuntimeFolder").GetValue(null,null);
            if(runtime.StartsWith(pluginFolder+Path.DirectorySeparatorChar,StringComparison.OrdinalIgnoreCase))throw new Exception("Native runtime is still inside MMC's recursive plugin scan");
            Directory.CreateDirectory(runtime);
            foreach(string name in new[]{"DynamicGamedayAtmosphereNative-"+release+".dll","LoadDynamicGamedayAtmosphere.exe","DynamicGamedayAtmosphere.ini","DgaDiagnostics.exe"}){
                if((direct||external)&&name=="LoadDynamicGamedayAtmosphere.exe")continue;
                if(plugin.GetManifestResourceStream("Attendance."+name)==null)throw new Exception("Missing resource "+name);
                extract.Invoke(null,new object[]{name,name.EndsWith(".ini")});
                if(name=="DgaDiagnostics.exe"){if(!File.Exists(Path.Combine(pluginFolder,"DynamicGamedayAtmosphere","Tools",name)))throw new Exception("Diagnostics window missing");continue;}
                if(!File.Exists(name.EndsWith(".ini")?Path.Combine(pluginFolder,"DynamicGamedayAtmosphere",name):Path.Combine(runtime,name)))throw new Exception("Missing extracted file "+name);
            }
            if(direct&&Directory.GetFiles(pluginFolder,"LoadDynamicGamedayAtmosphere.exe",SearchOption.AllDirectories).Length!=0)throw new Exception("Direct variant extracted a loader EXE");
            var scanned=Directory.GetFiles(pluginFolder,"*.dll",SearchOption.AllDirectories);
            if(scanned.Length!=1||!String.Equals(scanned[0],Path.Combine(pluginFolder,"DynamicGamedayAtmosphere.dll"),StringComparison.OrdinalIgnoreCase))throw new Exception("MMC would discover an extra native DLL on restart");
            // Extracting again simulates the next MMC start: payload is reusable,
            // with no native DLL appearing inside the plugin scan tree.
            extract.Invoke(null,new object[]{"DynamicGamedayAtmosphereNative-"+release+".dll",false});
            if(Directory.GetFiles(pluginFolder,"*.dll",SearchOption.AllDirectories).Length!=1)throw new Exception("Restart extraction changed plugin discovery");
            Console.WriteLine("PASS: DLL-only fixture, automatic extraction outside Plugins, repeat extraction and recursive discovery find only the managed adapter; no game loaded");
            if(external){
                string loader=Path.Combine((string)action.GetProperty("RuntimeFolder").GetValue(null,null),"LoadDynamicGamedayAtmosphere.exe");
                var verify=action.GetMethod("VerifyExternalLoader",BindingFlags.Static|BindingFlags.NonPublic);
                verify.Invoke(null,new object[]{loader});byte[] original=File.ReadAllBytes(loader);
                try{
                    File.WriteAllBytes(loader,new byte[]{1,2,3});bool refused=false;
                    try{verify.Invoke(null,new object[]{loader});}catch(TargetInvocationException e){refused=e.InnerException is InvalidDataException;}
                    if(!refused)throw new Exception("External loader accepted a different hash");
                    File.Delete(loader);refused=false;
                    try{verify.Invoke(null,new object[]{loader});}catch(TargetInvocationException e){refused=e.InnerException is FileNotFoundException;}
                    if(!refused)throw new Exception("External loader accepted a missing file");
                }finally{File.WriteAllBytes(loader,original);}
                verify.Invoke(null,new object[]{loader});Console.WriteLine("PASS: separate loader hash, missing-file refusal and restoration; helper never executed");
            }
            action.GetMethod("Note",BindingFlags.Static|BindingFlags.NonPublic).Invoke(null,new object[]{null,"Package check passed"});
            if(!File.Exists(Path.Combine(pluginFolder,"DynamicGamedayAtmosphere","Logs","launcher.log")))throw new Exception("Launcher log is outside the plugin Logs folder");
            string log=Path.Combine(pluginFolder,"DynamicGamedayAtmosphere","Logs","launcher.log");
            File.WriteAllBytes(log,new byte[2*1024*1024]);
            action.GetMethod("Note",BindingFlags.Static|BindingFlags.NonPublic).Invoke(null,new object[]{null,"After size reset"});
            if(new FileInfo(log).Length>1024||!File.ReadAllText(log).Contains("After size reset")||File.Exists(log+".1"))throw new Exception("Launcher log cap failed");
            // MMC's logger must still receive errors when the private file is
            // locked, including a watcher started earlier by registration.
            CheckLoggerFallback(action,log);
            Console.WriteLine("PASS: MMC ABI, metadata, resources, extraction, nested launcher log, size reset and locked-file MMC logger fallback");return 0;
        }catch(Exception e){Console.Error.WriteLine(e);return 1;}
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    static void CheckLoggerFallback(Type action,string log){
        // Load the typed SDK interface only after Main installs the selected
        // MMC host's AssemblyResolve handler.
        var logger=new CapturedLogger();
        action.GetField("hostLogger",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,logger);
        using(var held=new FileStream(log,FileMode.Open,FileAccess.Read,FileShare.Read))
            action.GetMethod("Note",BindingFlags.Static|BindingFlags.NonPublic).Invoke(null,new object[]{null,"Blocked log fallback"});
        if(logger.Last==null||!logger.Last.Contains("Blocked log fallback")||!logger.Last.Contains("Log file unavailable"))throw new Exception("MMC logger fallback lost when private log failed");
    }
}
sealed class CapturedLogger : FrostySdk.Interfaces.ILogger {
    public string Last;
    public void Log(string text,params object[] args){Last=text;}
    public void LogWarning(string text,params object[] args){Last=text;}
    public void LogError(string text,params object[] args){Last=text;}
}
