using System;
using System.Drawing;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows.Forms;
namespace DynastyCOT.Collection {
 internal static class CollectionProgram {
  internal static Catalog Load(){using(var stream=typeof(CollectionProgram).Assembly.GetManifestResourceStream("Collection.catalog.json"))using(var bytes=new MemoryStream()){stream.CopyTo(bytes);if(Paths.Hash(bytes.ToArray())!=CollectionInfo.CatalogSha256)throw new IOException("Embedded catalog failed integrity verification.");var catalog=Json.Serializer().Deserialize<Catalog>(System.Text.Encoding.UTF8.GetString(bytes.ToArray()));catalog.Validate();return catalog;}}
  static void CollectFonts(Control root,List<Tuple<Control,Font>> result){result.Add(Tuple.Create(root,root.Font));foreach(Control child in root.Controls)CollectFonts(child,result);}
  static void ScalePreview(CollectionForm form,float scale){form.AutoScaleMode=AutoScaleMode.None;var fonts=new List<Tuple<Control,Font>>();CollectFonts(form,fonts);form.Scale(new SizeF(scale,scale));foreach(var item in fonts)item.Item1.Font=new Font(item.Item2.FontFamily,item.Item2.SizeInPoints*scale,item.Item2.Style);form.SetPreviewScale(scale);}
  static Fixture FixtureMode(string[] args){var fixture=new Fixture(args[1],args[2]);typeof(Ace.DynastyCOT.UserStorage).GetField("current",System.Reflection.BindingFlags.Static|System.Reflection.BindingFlags.NonPublic).SetValue(null,new Ace.DynastyCOT.UserStore(Path.Combine(fixture.Parent,"LayoutProfile","Ace","DynastyCOT")));return fixture;}
  static string Output(Fixture fixture,string path){path=Path.GetFullPath(path);if(!path.StartsWith(fixture.Parent+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("Evidence must stay inside the fixture parent.");Directory.CreateDirectory(path);return path;}
  [STAThread]static int Main(string[] args){try{
   DpiSupport.Initialize();Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);var catalog=Load();
   if(args.Length==0){Application.Run(new CollectionForm(catalog));return 0;}
   if(args.Length==2&&args[0]=="--manager"){Application.Run(new CollectionForm(catalog,null,args[1]));return 0;}
   if(args.Length<3)throw new IOException("Marked fixture arguments required.");var fixture=FixtureMode(args);
   if(args[0]=="--preview-fixture"){var form=new CollectionForm(catalog,fixture);if(args.Length==4)ScalePreview(form,float.Parse(args[3],System.Globalization.CultureInfo.InvariantCulture)/100);Application.Run(form);return 0;}
   if(args[0]=="--preview-updates"){var choices=catalog.Modules.ToDictionary(m=>m.Id,m=>new List<ModuleRelease>{m.Current});Application.Run(new UpdateForm(catalog,new CollectionInstaller(catalog,fixture).Inspect(fixture.Root),choices,(i,r)=>{}));return 0;}
   if(args.Length==4&&args[0]=="--layout-check"){
    string output=Output(fixture,args[3]);int count=0;foreach(float scale in new[]{1f,1.25f,1.5f,1.75f,2f})foreach(var screen in new[]{new Size(1366,768),new Size(1920,1080),new Size(1280,720)})using(var form=new CollectionForm(catalog,fixture)){
     form.ShowInTaskbar=false;form.Show();ScalePreview(form,scale);form.MinimumSize=Size.Empty;form.ClientSize=new Size(Math.Min((int)(1020*scale),screen.Width-48),Math.Min((int)(650*scale),screen.Height-80));Application.DoEvents();try{form.AuditLayout();}catch{using(var failed=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(failed,new Rectangle(Point.Empty,failed.Size));failed.Save(Path.Combine(output,"FAILED-"+(scale*100).ToString("0")+"-"+screen.Width+".png"));}throw;}using(var image=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(image,new Rectangle(Point.Empty,image.Size));image.Save(Path.Combine(output,"collection-"+(scale*100).ToString("0")+"-"+screen.Width+".png"));}form.Close();count++;
    }Console.WriteLine("PASS "+count+" actual-window layout fixtures: 100/125/150/175/200 percent, 1280x720/1366x768/1920x1080; module-only scrolling when space requires it. DPI values are emulated; physical mixed-monitor movement remains live acceptance.");return 0;
   }
   if(args.Length==4&&args[0]=="--ui-benchmark"){
    string output=Output(fixture,args[3]);var samples=new List<object>();
    for(int i=0;i<5;i++){var watch=System.Diagnostics.Stopwatch.StartNew();using(var form=new CollectionForm(catalog,fixture)){form.Show();Application.DoEvents();double visible=watch.Elapsed.TotalMilliseconds;while(!form.InspectionReady&&watch.ElapsedMilliseconds<10000){Application.DoEvents();System.Threading.Thread.Sleep(1);}form.AuditLayout();samples.Add(new{operation="window",ms=visible,readyMs=watch.Elapsed.TotalMilliseconds});form.Close();}}
    var engine=new CollectionInstaller(catalog,fixture);for(int i=0;i<5;i++){var watch=System.Diagnostics.Stopwatch.StartNew();engine.Inspect(fixture.Root);samples.Add(new{operation="inspect",ms=watch.Elapsed.TotalMilliseconds});}
    var choices=catalog.Modules.ToDictionary(m=>m.Id,m=>new List<ModuleRelease>{m.Current});for(int i=0;i<5;i++){var inv=engine.Inspect(fixture.Root);var watch=System.Diagnostics.Stopwatch.StartNew();using(var updates=new UpdateForm(catalog,inv,choices,(n,r)=>{})){updates.Show();Application.DoEvents();updates.Audit();samples.Add(new{operation="updates",ms=watch.Elapsed.TotalMilliseconds});updates.Close();}}
    using(var notes=new ReleaseNotesView()){notes.Render("# Heading\n- **Bold** and *emphasis* with `code`\n[Safe](https://github.com/ArtVsTheWorld) [Unsafe](javascript:alert)\n![remote](https://example.invalid/image.png)\n<iframe>plain text</iframe>");if(notes.Text.Contains("# Heading")||notes.Text.Contains("**Bold**")||!notes.Text.Contains("Bold")||!notes.Text.Contains("[Image: remote]"))throw new IOException("Markdown rendering regression.");}
    File.WriteAllBytes(Path.Combine(output,"ui-performance.json"),Json.Bytes(new{samples=samples,memoryBytes=System.Diagnostics.Process.GetCurrentProcess().PeakWorkingSet64}));Console.WriteLine("PASS native update rendering and fixture window/inspection/update timings.");return 0;
   }
   throw new IOException("Unknown installer argument.");
  }catch(Exception error){if(args.Length==0)MessageBox.Show(error.Message,"Dynasty COT",MessageBoxButtons.OK,MessageBoxIcon.Error);else Console.Error.WriteLine(error);return 1;}}
 }
}
