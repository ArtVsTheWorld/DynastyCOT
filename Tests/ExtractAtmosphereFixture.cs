// Extract resources only, into a marked installer fixture. No engines run.
using System;
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions;
class ExtractAtmosphereFixture {
 static int Main(string[] args){
  try{
   string root=Path.GetFullPath(args[0]),parent=Path.GetFullPath(args[1]).TrimEnd('\\')+"\\";
   if(!root.StartsWith(parent,StringComparison.OrdinalIgnoreCase)||!File.Exists(Path.Combine(root,".cot-fixture")))throw new IOException("Fixture required");
   var assembly=Assembly.LoadFrom(Path.Combine(root,"Plugins","DynastyCOT.dll"));
   foreach(string resource in assembly.GetManifestResourceNames()){
    var match=Regex.Match(resource,@"^Attendance\.DynamicGamedayAtmosphereNative-(\d+\.\d+\.\d+)\.dll$");
    if(!match.Success)continue;
    string folder=Path.Combine(root,"DGA-Runtime","Runtime-"+match.Groups[1].Value);Directory.CreateDirectory(folder);
    foreach(string name in new[]{"DynamicGamedayAtmosphereNative-"+match.Groups[1].Value+".dll","DgaDiagnostics.exe"})
     using(var input=assembly.GetManifestResourceStream("Attendance."+name))using(var output=File.Create(Path.Combine(folder,name)))input.CopyTo(output);
    return 0;
   }throw new IOException("Atmosphere resource missing");
  }catch(Exception error){Console.Error.WriteLine(error);return 1;}
 }
}
