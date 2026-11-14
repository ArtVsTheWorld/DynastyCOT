"""Isolated adapter verification; never starts an MMC watcher or attaches to a game."""
from pathlib import Path
import hashlib, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[3]
source=ROOT/'Modules/DynastyEnhancer/Source/MmcRecruitingPlugin.cs'
s=source.read_text()
assert 'Process.Start(' not in s and 'LoadDynastyEnhancer.exe' not in s
assert 'ManagedRuntimeLoader.Load(game.Id,Path.Combine(RuntimeFolder,nativeFile),Note)' in s
assert s.index('attempts[key]=3;') < s.index('ManagedRuntimeLoader.Load(')
assert 'var key=game.Id+":"+game.StartTime.ToUniversalTime().Ticks+":"+nativeFile' in s
assert 'Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),"Logs")' in s
assert 'SHA256.Create()' in s and 'Payload path escapes runtime folder' in s
bridge=(ROOT/'Modules/DynastyEnhancer/Source/AnalyticsMenuBridge.h').read_text()
assert 'if(name==L"BeyondTheFieldNative-v103.dll")return own.parent_path()/L"BeyondTheFieldRecruitFog-v1.dll";' in bridge
assert 'L"DynastyEnhancerNative-cot-"' in bridge and 'cotanalytics::compatible(*value)' in bridge
with tempfile.TemporaryDirectory(prefix='btf-launcher-',dir=ROOT/'Build') as temp:
    d=Path(temp);runtime=d/'BeyondTheField/Runtime-0.1.105';runtime.mkdir(parents=True)
    payload=runtime/'BeyondTheFieldNative-v103.dll';payload.write_bytes(b'isolated payload')
    manifest=d/'payload.txt';manifest.write_text(hashlib.sha256(payload.read_bytes()).hexdigest()+'|'+payload.name+'\n')
    stubs=d/'Stubs.cs';stubs.write_text(r'''using System;using System.Threading;
namespace Ace.DynastyCOT{public static class UserStorage {public static string Folder(string module){return System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory,module);}}}
namespace FrostySdk.Interfaces {public interface ILogger{}}
namespace Frosty.Core {public enum PluginManagerType{ModManager} public class ExecutionAction {public virtual Action<FrostySdk.Interfaces.ILogger,PluginManagerType,CancellationToken> PreLaunchAction{get{return null;}}public virtual Action<FrostySdk.Interfaces.ILogger,PluginManagerType,CancellationToken> PostLaunchAction{get{return null;}}}}
namespace Frosty.Core.Attributes {
public class PluginVersionAttribute:Attribute{public PluginVersionAttribute(string x){}}
public class PluginAuthorAttribute:Attribute{public PluginAuthorAttribute(string x){}}
public class PluginDisplayNameAttribute:Attribute{public PluginDisplayNameAttribute(string x){}}
public class RegisterExecutionActionAttribute:Attribute{public RegisterExecutionActionAttribute(Type x){}}}
namespace DynamicGamedayAtmosphere {internal static class ManagedRuntimeLoader {internal static void Load(int pid,string file,Action<string> note){throw new Exception("Fixture must not start runtime");}}}
class Check {static int Main(){
var t=typeof(DynastyEnhancer.LaunchAction);DynastyEnhancer.LaunchAction.Prepare();
foreach(string x in new[]{"../escape","C:/escape","a/../../../escape"}){bool reject=false;try{DynastyEnhancer.LaunchAction.PayloadPath(x);}catch(System.IO.IOException){reject=true;}if(!reject)throw new Exception("path accepted");}
var file=DynastyEnhancer.LaunchAction.PayloadPath(DynastyEnhancer.LaunchAction.NativeFile);System.IO.File.AppendAllText(file,"tamper");bool bad=false;try{DynastyEnhancer.LaunchAction.Prepare();}catch(System.IO.IOException){bad=true;}if(!bad)throw new Exception("tamper accepted");
var note=t.GetMethod("Note",System.Reflection.BindingFlags.Static|System.Reflection.BindingFlags.NonPublic);note.Invoke(null,new object[]{"fixture"});
string folder=System.IO.Path.Combine(Ace.DynastyCOT.UserStorage.Folder("BeyondTheField"),"Logs");string log=System.IO.Path.Combine(folder,"BeyondTheField-launcher.log");if(!System.IO.File.ReadAllText(log).Contains("fixture"))throw new Exception("log absent");
System.IO.File.WriteAllText(log,new string('x',1024*1024+1));note.Invoke(null,new object[]{"rotated"});if(!System.IO.File.Exists(log+".1"))throw new Exception("rotation missing");
using(var locked=System.IO.File.Open(log,System.IO.FileMode.Open,System.IO.FileAccess.ReadWrite,System.IO.FileShare.None))note.Invoke(null,new object[]{"locked diagnostic cannot stop watcher"});
if(System.IO.File.Exists(System.IO.Path.Combine(DynastyEnhancer.LaunchAction.PluginsFolder,"DynastyEnhancer-launcher.log")))throw new Exception("loose log");
Console.WriteLine("PASS: actual adapter manifest/tamper/path guards, nested launcher log/rotation; no process attachment.");return 0;}}
''')
    exe=d/'Check.exe'
    command=[r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe','/nologo','/target:exe','/platform:x64','/reference:System.Web.Extensions.dll','/out:'+str(exe),'/resource:'+str(manifest)+',DynastyEnhancer.payload-sha256.txt',str(source),str(stubs)]
    subprocess.run(command,check=True,capture_output=True,text=True)
    subprocess.run([str(exe)],check=True)
print('PASS: BTF branded native pair, managed loading without EXE, no uncertain-completion retry, sibling owner/build guards preserved.')
