using System;using System.IO;using System.Reflection;using System.Windows.Forms;
using System.Drawing;
using DynamicGamedayAtmosphere.Diagnostics;
class DiagnosticWindowTests {
 static object Field(Window w,string name){return typeof(Window).GetField(name,BindingFlags.NonPublic|BindingFlags.Instance).GetValue(w);}
 static void Check(bool okay){if(!okay)throw new Exception("Diagnostics window regression");}
 [STAThread]static int Main(){
  string root=Path.Combine(Path.GetTempPath(),"DgaWindowTest-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(Path.Combine(root,"Logs"));
  string status=Path.Combine(root,"Logs","status.txt");File.WriteAllText(status,"Dynamic Gameday Atmosphere 1.55.0\nGame process ID: 42\nAudio stadium fullness: 75.4% of mapped seating\nDiagnostic preview: idle\n");
  try{
   Check(Program.DefaultData(Path.Combine(root,"DynamicGamedayAtmosphere","Tools"))==Path.Combine(root,"DynamicGamedayAtmosphere"));
   Check(Program.DefaultData(root)==Path.Combine(root,"DynamicGamedayAtmosphere"));
   using(var w=new Window(root)){
    Check(((Label)Field(w,"fullness")).Text=="Stadium Fullness: 75.4%"&&((ComboBox)Field(w,"preset")).Items.Count==10);
    Check(((NumericUpDown)Field(w,"seconds")).Maximum==60&&((CheckBox)Field(w,"sound")).Checked&&!((CheckBox)Field(w,"camera")).Checked);
    foreach(Control control in w.Controls)if(control is Label&&control.Text.Length>0){Size size=TextRenderer.MeasureText(control.Text,control.Font,new Size(control.Width,10000),TextFormatFlags.WordBreak|TextFormatFlags.NoPrefix);Check(size.Height<=control.Height+2);}
    typeof(Window).GetMethod("Send",BindingFlags.NonPublic|BindingFlags.Instance).Invoke(w,new object[]{false});
    string command=File.ReadAllText(Path.Combine(root,"Diagnostics","request-42.txt"));string[] parts=command.Trim().Split(' ');
    Check(parts.Length==8&&parts[0]=="DGA-DIAGNOSTIC-1"&&parts[1]=="42"&&parts[4]=="lineup"&&parts[5]=="20"&&parts[6]=="1"&&parts[7]=="0");
    Check(!File.Exists(Path.Combine(root,"DynamicGamedayAtmosphere.ini")));
    File.SetLastWriteTimeUtc(status,DateTime.UtcNow-TimeSpan.FromMinutes(1));
    typeof(Window).GetMethod("RefreshState",BindingFlags.NonPublic|BindingFlags.Instance).Invoke(w,null);
    Check(!(bool)Field(w,"fresh")&&((Label)Field(w,"fullness")).Text.Contains("waiting"));
   }
   Console.WriteLine("PASS: diagnostics window controls, fullness/status parsing, matching request protocol, bounded duration, local-only files and stale-runtime rejection (no visible window or game changes).");return 0;
  }catch(Exception error){Console.Error.WriteLine(error);return 1;}
 }
}
