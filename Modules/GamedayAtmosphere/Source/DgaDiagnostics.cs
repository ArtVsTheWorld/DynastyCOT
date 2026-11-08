using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Globalization;
using System.Windows.Forms;
using System.Drawing;
using System.Runtime.InteropServices;

namespace DynamicGamedayAtmosphere.Diagnostics {
    sealed class Window : Form {
        readonly string data;
        readonly ComboBox preset=new ComboBox();readonly NumericUpDown seconds=new NumericUpDown();
        readonly CheckBox sound=new CheckBox(),camera=new CheckBox();readonly Label fullness=new Label(),state=new Label();
        readonly Timer timer=new Timer();int pid;bool fresh;
        bool startKey,stopKey,hotkeysAttempted;
        [DllImport("user32.dll")]static extern bool RegisterHotKey(IntPtr window,int id,uint modifiers,uint key);
        [DllImport("user32.dll")]static extern bool UnregisterHotKey(IntPtr window,int id);
        public Window(string folder){
            data=folder;Text="Gameday Atmosphere - Diagnostics";ClientSize=new Size(540,394);Font=new Font("Segoe UI",9);FormBorderStyle=FormBorderStyle.FixedDialog;MaximizeBox=false;
            var tip=new Label{Left=18,Top=15,Width=505,Height=52,Text="Choose a preset, return to the game and press F8 while lined up and unpaused. F9 stops it. Disable the play clock in game settings. Tests do not change clocks, scores, attendance or home-field advantage."};Controls.Add(tip);
            fullness.SetBounds(18,73,505,25);fullness.Font=new Font(Font.FontFamily,12,FontStyle.Bold);Controls.Add(fullness);
            preset.SetBounds(18,109,270,25);preset.DropDownStyle=ComboBoxStyle.DropDownList;
            preset.Items.AddRange(new object[]{"Roar","Swell","Cheer","Conversion","Defense","Disappointment","Lineup","Critical","Touchdown","Eruption"});preset.SelectedIndex=6;Controls.Add(preset);
            seconds.SetBounds(305,109,60,25);seconds.Minimum=5;seconds.Maximum=60;seconds.Value=20;Controls.Add(seconds);Controls.Add(new Label{Left=370,Top=113,Width=150,Text="seconds, including fade"});
            sound.SetBounds(18,147,160,24);sound.Text="Sound";sound.Checked=true;camera.SetBounds(185,147,180,24);camera.Text="Camera movement";Controls.Add(sound);Controls.Add(camera);
            var start=new Button{Left=18,Top=184,Width=150,Height=32,Text="Start preview"};start.Click+=(s,e)=>Send(false);Controls.Add(start);
            var stop=new Button{Left=185,Top=184,Width=150,Height=32,Text="Stop preview"};stop.Click+=(s,e)=>Send(true);Controls.Add(stop);
            var logs=new Button{Left=352,Top=184,Width=165,Height=32,Text="Open logs"};logs.Click+=(s,e)=>{try{System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(Path.Combine(data,"Logs")){UseShellExecute=true});}catch(Exception error){state.Text=error.Message;}};Controls.Add(logs);
            var report=new Button{Left=18,Top=226,Width=499,Height=30,Text="Save read-only game compatibility report"};report.Click+=(s,e)=>{
                try{string path=GameCompatibility.Capture(data);MessageBox.Show("Read-only report saved:\n\n"+path+"\n\nThis file can be shared for compatibility troubleshooting.",Text);}
                catch(Exception error){state.Text="Could not save report: "+error.Message;}
            };Controls.Add(report);
            state.SetBounds(18,266,505,54);Controls.Add(state);
            Controls.Add(new Label{Left=18,Top=331,Width=505,Height=52,Text="Pause, replay, a snap or a new game cancels the test. One-shot cheers finish naturally. Current settings and volume limits still apply. F8/F9 work while this window is open."});
            timer.Interval=1000;timer.Tick+=(s,e)=>RefreshState();timer.Start();FormClosing+=(s,e)=>{if(fresh)Send(true);};RefreshState();
        }
        protected override void OnShown(EventArgs e){base.OnShown(e);hotkeysAttempted=true;startKey=RegisterHotKey(Handle,1,0x4000,(uint)Keys.F8);stopKey=RegisterHotKey(Handle,2,0x4000,(uint)Keys.F9);RefreshState();}
        protected override void WndProc(ref Message message){if(message.Msg==0x0312){if(message.WParam.ToInt32()==1)Send(false);else if(message.WParam.ToInt32()==2)Send(true);}base.WndProc(ref message);}
        void RefreshState(){
            try{
                string path=Path.Combine(data,"Logs","status.txt");fresh=File.Exists(path)&&DateTime.UtcNow-File.GetLastWriteTimeUtc(path)<TimeSpan.FromSeconds(4);
                if(!fresh){fullness.Text="Stadium fullness: waiting for the game";state.Text="Start MMC and an offline game with the current working DLL loaded.";pid=0;return;}
                string text;using(var file=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.ReadWrite|FileShare.Delete))using(var reader=new StreamReader(file))text=reader.ReadToEnd();
                Match process=Regex.Match(text,@"Game process ID\s*:\s*(\d+)");pid=process.Success?Int32.Parse(process.Groups[1].Value,CultureInfo.InvariantCulture):0;
                Match crowd=Regex.Match(text,@"Audio stadium fullness\s*:\s*([\d.]+)%");
                fullness.Text=crowd.Success?"Stadium Fullness: "+crowd.Groups[1].Value+"%":"Stadium fullness: waiting for mapped seating";
                Match current=Regex.Match(text,@"Diagnostic preview\s*:\s*([^\r\n]+)");state.Text=current.Success?current.Groups[1].Value:"This DLL does not support repeatable previews. Load the current working build.";
                fresh=fresh&&current.Success&&pid>0;
                if(hotkeysAttempted&&(!startKey||!stopKey))state.Text+="\nA shortcut is in use elsewhere; use the buttons.";
            }catch(IOException){fresh=false;}catch(Exception error){state.Text=error.Message;fresh=false;}
        }
        void Send(bool stop){
            if(!fresh||pid<=0){if(!stop)MessageBox.Show("Wait for a fresh status from the current working DLL.",Text);return;}
            if(!stop&&!sound.Checked&&!camera.Checked){MessageBox.Show("Select Sound or Camera movement.",Text);return;}
            try{
                string dir=Path.Combine(data,"Diagnostics");Directory.CreateDirectory(dir);
                string temporary=Path.Combine(dir,"request-"+Guid.NewGuid().ToString("N")+".tmp"),destination=Path.Combine(dir,"request-"+pid.ToString(CultureInfo.InvariantCulture)+".txt");
                ulong token=BitConverter.ToUInt64(Guid.NewGuid().ToByteArray(),0);if(token==0)token=1;
                long utc=(DateTime.UtcNow.Ticks-621355968000000000L)/10000;
                string command=String.Format(CultureInfo.InvariantCulture,"DGA-DIAGNOSTIC-1 {0} {1} {2:x16} {3} {4} {5} {6}\n",pid,utc,token,stop?"stop":preset.SelectedItem.ToString().ToLowerInvariant(),(int)seconds.Value,sound.Checked?1:0,camera.Checked?1:0);
                File.WriteAllText(temporary,command,new UTF8Encoding(false));
                if(File.Exists(destination))File.Replace(temporary,destination,null);else File.Move(temporary,destination);
                state.Text=stop?"Stop requested; restoring normal atmosphere.":"Preview requested. Stay lined up without snapping.";
            }catch(Exception error){state.Text="Could not send request: "+error.Message;}
        }
        protected override void Dispose(bool disposing){if(disposing){timer.Dispose();if(startKey)UnregisterHotKey(Handle,1);if(stopKey)UnregisterHotKey(Handle,2);startKey=stopKey=false;}base.Dispose(disposing);}
    }
    static class Program {
        internal static string DefaultData(string directory){
            var location=new DirectoryInfo(Path.GetFullPath(directory));
            return location.Name.Equals("Tools",StringComparison.OrdinalIgnoreCase)&&location.Parent!=null&&location.Parent.Name.Equals("DynamicGamedayAtmosphere",StringComparison.OrdinalIgnoreCase)?location.Parent.FullName:Path.Combine(location.FullName,"DynamicGamedayAtmosphere");
        }
        [STAThread]static void Main(string[] args){
            if(args.Length>0&&args[0]=="--game-report"){
                string target=args.Length==2?Path.GetFullPath(args[1]):DefaultData(AppDomain.CurrentDomain.BaseDirectory);
                try{GameCompatibility.Capture(target);}catch(Exception error){MessageBox.Show(error.Message,"DGA compatibility report");Environment.ExitCode=1;}return;
            }
            string data=args.Length==1?Path.GetFullPath(args[0]):DefaultData(AppDomain.CurrentDomain.BaseDirectory);
            Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);Application.Run(new Window(data));
        }
    }
}
