"""SDK-free persistence/lifecycle checks. Never attaches to a running game."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'Build/SupporterRecruitPass/Safety';OUT.mkdir(parents=True,exist_ok=True)
code=OUT/'SafeCore.cs';code.write_text('''using System;using System.IO;using System.Reflection;class SafeCore {static void Main(string[] args){typeof(Ace.DynastyCOT.UserStorage).GetField("current",BindingFlags.Static|BindingFlags.NonPublic).SetValue(null,new Ace.DynastyCOT.UserStore(Path.Combine(args[0],"UserData")));try{typeof(CoreChecks).GetMethod("Main",BindingFlags.Static|BindingFlags.NonPublic).Invoke(null,new object[]{new string[]{Path.Combine(args[0],"Fixture")}});}catch(TargetInvocationException e){throw e.InnerException;}}}''')
exe=OUT/'ExtraCore.exe';csc=r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe'
# The imported standalone migration tests expect destructive portable-folder
# moves. Current shared-AppData migration has its own UserStorageChecks suite.
original=(ROOT/'Modules/ExtraDynastySettings/Tools/CoreChecks.cs').read_text();core=OUT/'CoreChecks.cs';core.write_text(original[:original.index('        string migration=')]+original[original.index('        Env env=new Env();'):])
subprocess.run([csc,'/nologo','/platform:x64','/main:SafeCore','/out:'+str(exe),str(code),str(ROOT/'Core/UserStorage.cs'),str(ROOT/'Modules/ExtraDynastySettings/Plugin/LaunchCoordinator.cs'),str(ROOT/'Modules/ExtraDynastySettings/Plugin/ClockPreferences.cs'),str(core)],check=True,capture_output=True)
print(subprocess.check_output([str(exe),str(OUT/'ExtraProfile')],text=True),flush=True)
module=ROOT/'Modules/DrivePractice'
common=[ROOT/'Core/UserStorage.cs',*(module/'Plugin'/name for name in ('DriveEngine.cs','NativeRuntime.cs','NativeCompatibility.cs','DynastyControllerCompatibility.cs','NativeDrivePolicy.cs','SessionStats.cs','DefenseChoice.cs','Diagnostics.cs'))]
for name in ('DriveChecks','NativeDriveChecks','DefenseChecks','OpponentChecks','SessionStatsChecks','ScrimmageChecks','ModeChecks','StartChecks','FirstDownLineChecks','DiagnosticsChecks'):
 test=OUT/(name+'.exe');subprocess.run([csc,'/nologo','/platform:x64','/reference:System.IO.Compression.dll','/out:'+str(test),*(str(p) for p in common),str(module/'Tools'/(name+'.cs'))],check=True,capture_output=True)
 print(name,subprocess.check_output([str(test)],cwd=OUT,text=True).strip(),flush=True)
