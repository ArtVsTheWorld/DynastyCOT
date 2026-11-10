using System;
using System.Drawing;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;
namespace DynastyCOT.Collection {
 internal static class CollectionProgram {
  internal static Catalog Load(){using(var stream=typeof(CollectionProgram).Assembly.GetManifestResourceStream("Collection.catalog.json"))using(var bytes=new MemoryStream()){stream.CopyTo(bytes);if(Paths.Hash(bytes.ToArray())!=CollectionInfo.CatalogSha256)throw new IOException("Embedded catalog failed integrity verification.");var catalog=Json.Serializer().Deserialize<Catalog>(System.Text.Encoding.UTF8.GetString(bytes.ToArray()));catalog.Validate();return catalog;}}
  static void CollectFonts(Control root,List<Tuple<Control,Font>> result){result.Add(Tuple.Create(root,root.Font));foreach(Control child in root.Controls)CollectFonts(child,result);}
  [STAThread]static int Main(string[] args){try{Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);var catalog=Load();if(args.Length==0){Application.Run(new CollectionForm(catalog));return 0;}if(args.Length==3&&args[0]=="--preview-fixture"){Application.Run(new CollectionForm(catalog,new Fixture(args[1],args[2])));return 0;}if(args.Length==4&&args[0]=="--layout-check"){
    var fixture=new Fixture(args[1],args[2]);string output=Path.GetFullPath(args[3]);if(!output.StartsWith(fixture.Parent+"\\",StringComparison.OrdinalIgnoreCase))throw new IOException("Layout captures must stay inside the fixture parent.");Directory.CreateDirectory(output);foreach(float scale in new[]{1f,1.25f,1.5f})using(var form=new CollectionForm(catalog,fixture)){form.ShowInTaskbar=false;form.Show();form.AutoScaleMode=AutoScaleMode.None;var fonts=new List<Tuple<Control,Font>>();CollectFonts(form,fonts);form.Scale(new SizeF(scale,scale));foreach(var item in fonts)item.Item1.Font=new Font(item.Item2.FontFamily,item.Item2.SizeInPoints*scale,item.Item2.Style);Application.DoEvents();form.AuditLayout();using(var image=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(image,new Rectangle(Point.Empty,image.Size));image.Save(Path.Combine(output,"collection-"+(scale*100).ToString("0")+".png"));}form.Close();}Console.WriteLine("PASS actual collection window: four compact cards, fitted descriptions, footer visible at 100/125/150 percent; no app-ui image.");return 0;
   }throw new IOException("Unknown installer argument.");}catch(Exception e){if(args.Length==0)MessageBox.Show(e.Message,"Dynasty COT",MessageBoxButtons.OK,MessageBoxIcon.Error);else Console.Error.WriteLine(e);return 1;}}
 }
}
