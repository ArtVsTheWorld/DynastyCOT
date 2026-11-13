using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using DynastyCOT.Setup;
namespace DynastyCOT.Collection {
 internal sealed class UpdateForm:Form {
  readonly ListBox modules=new ListBox();readonly ComboBox versions=new ComboBox();
  readonly Label installed=new Label(),compatibility=new Label();readonly ReleaseNotesView notes=new ReleaseNotesView();
  readonly RoundedButton use=new RoundedButton();
  internal UpdateForm(Catalog catalog,Inventory inventory,Dictionary<string,List<ModuleRelease>> choices,Action<int,ModuleRelease> choose){
   Text="Choose module versions";Font=new Font("Segoe UI",10);BackColor=Theme.Background;ForeColor=Theme.Text;ClientSize=new Size(900,580);MinimumSize=new Size(660,410);AutoScaleMode=AutoScaleMode.Dpi;AutoScaleDimensions=new SizeF(96,96);StartPosition=FormStartPosition.CenterParent;
   var grid=new TableLayoutPanel{Dock=DockStyle.Fill,Padding=new Padding(16),ColumnCount=2,RowCount=1};grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,25));grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,75));Controls.Add(grid);
   modules.Dock=DockStyle.Fill;modules.BackColor=Theme.Surface;modules.ForeColor=Theme.Text;modules.BorderStyle=BorderStyle.None;modules.Items.AddRange(catalog.Modules.Select(m=>m.Title).ToArray());grid.Controls.Add(modules,0,0);
   var body=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=5,Padding=new Padding(14,0,0,0)};body.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));foreach(float height in new[]{28f,34f,58f,-1f,56f})body.RowStyles.Add(new RowStyle(height<0?SizeType.Percent:SizeType.Absolute,height<0?100:height));grid.Controls.Add(body,1,0);
   installed.Dock=DockStyle.Fill;installed.TextAlign=ContentAlignment.MiddleLeft;body.Controls.Add(installed,0,0);versions.Dock=DockStyle.Fill;versions.DropDownStyle=ComboBoxStyle.DropDownList;versions.AccessibleName="Installer-compatible public releases";body.Controls.Add(versions,0,1);
   compatibility.Dock=DockStyle.Fill;compatibility.ForeColor=Theme.Muted;compatibility.Font=new Font(Font.FontFamily,9);body.Controls.Add(compatibility,0,2);notes.Dock=DockStyle.Fill;notes.Font=Font;body.Controls.Add(notes,0,3);
   var buttons=new FlowLayoutPanel{Dock=DockStyle.Fill,FlowDirection=FlowDirection.RightToLeft,WrapContents=false,Padding=new Padding(0,5,0,0)};var done=new RoundedButton{Text="Done",Width=85,Height=34,ForeColor=Theme.Text,BackColor=Theme.Surface};use.Text="Use this version";use.Primary=true;use.Size=new Size(180,34);use.ForeColor=Theme.Text;use.BackColor=Theme.Surface;buttons.Controls.Add(done);buttons.Controls.Add(use);body.Controls.Add(buttons,0,4);done.Click+=(s,e)=>Close();
   Action changed=()=>{var release=versions.SelectedItem as ModuleRelease;compatibility.Text=release==null?"Keep your installed version, or check again later.":release.Compatibility;notes.Render(release==null?"No installer-compatible public release was found.":release.Notes);use.Enabled=release!=null;use.Text="Use this version";};
   versions.SelectedIndexChanged+=(s,e)=>changed();modules.SelectedIndexChanged+=(s,e)=>{var module=catalog.Modules[modules.SelectedIndex];installed.Text=inventory==null?"Not installed":inventory.Status(module);versions.Items.Clear();versions.Items.AddRange(choices[module.Id].Cast<object>().ToArray());if(versions.Items.Count>0)versions.SelectedIndex=0;else changed();};
   use.Click+=(s,e)=>{if(modules.SelectedIndex<0||versions.SelectedItem==null)return;choose(modules.SelectedIndex,(ModuleRelease)versions.SelectedItem);use.Text="Selected for install";};modules.SelectedIndex=0;
   Shown+=(s,e)=>{var area=Screen.FromControl(this).WorkingArea;if(Width>area.Width-24||Height>area.Height-24){MinimumSize=Size.Empty;Size=new Size(Math.Min(Width,area.Width-24),Math.Min(Height,area.Height-24));}};
  }
  internal void Audit(){PerformLayout();if(notes.Width<200||notes.Height<80)throw new InvalidOperationException("Release notes pane is too small.");if(!use.Parent.ClientRectangle.Contains(use.Bounds))throw new InvalidOperationException("Update action is clipped.");}
 }
}
