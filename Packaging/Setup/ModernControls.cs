using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
namespace DynastyCOT.Setup {
 internal static class Theme {
  internal static readonly Color Background=Color.FromArgb(12,19,24),Surface=Color.FromArgb(24,34,41),Border=Color.FromArgb(61,77,87),Text=Color.FromArgb(241,246,244),Muted=Color.FromArgb(170,187,192),Green=Color.FromArgb(12,185,102);
  internal static GraphicsPath Shape(Rectangle r,int radius){var p=new GraphicsPath();int d=Math.Min(radius*2,Math.Min(r.Width,r.Height));if(d<2){p.AddRectangle(r);return p;}p.AddArc(r.Left,r.Top,d,d,180,90);p.AddArc(r.Right-d,r.Top,d,d,270,90);p.AddArc(r.Right-d,r.Bottom-d,d,d,0,90);p.AddArc(r.Left,r.Bottom-d,d,d,90,90);p.CloseFigure();return p;}
 }
 internal sealed class RoundedPanel:Panel {
  internal RoundedPanel(){DoubleBuffered=true;BackColor=Theme.Surface;Padding=new Padding(12);}
  protected override void OnPaintBackground(PaintEventArgs e){e.Graphics.Clear(Parent==null?Theme.Background:Parent.BackColor);}
  protected override void OnPaint(PaintEventArgs e){base.OnPaint(e);e.Graphics.SmoothingMode=SmoothingMode.AntiAlias;using(var path=Theme.Shape(new Rectangle(0,0,Width-1,Height-1),12))using(var brush=new LinearGradientBrush(ClientRectangle,BackColor,Color.FromArgb(Math.Max(0,BackColor.R-4),Math.Max(0,BackColor.G-4),Math.Max(0,BackColor.B-4)),90f))using(var pen=new Pen(Theme.Border)){e.Graphics.FillPath(brush,path);e.Graphics.DrawPath(pen,path);}}
 }
 internal sealed class RoundedButton:Button {
  bool hover;internal bool Primary;
  internal RoundedButton(){FlatStyle=FlatStyle.Flat;FlatAppearance.BorderSize=0;Cursor=Cursors.Hand;SetStyle(ControlStyles.UserPaint|ControlStyles.OptimizedDoubleBuffer|ControlStyles.AllPaintingInWmPaint,true);}
  protected override void OnMouseEnter(EventArgs e){hover=true;Invalidate();base.OnMouseEnter(e);}protected override void OnMouseLeave(EventArgs e){hover=false;Invalidate();base.OnMouseLeave(e);}
  protected override void OnPaint(PaintEventArgs e){e.Graphics.Clear(Parent==null?Theme.Background:Parent.BackColor);e.Graphics.SmoothingMode=SmoothingMode.AntiAlias;var color=Primary?Theme.Green:BackColor;if(hover&&Enabled)color=ControlPaint.Light(color,.12f);using(var p=Theme.Shape(new Rectangle(1,1,Width-3,Height-3),10))using(var b=new SolidBrush(color))using(var border=new Pen(Primary?Color.FromArgb(25,220,131):Theme.Border)){e.Graphics.FillPath(b,p);e.Graphics.DrawPath(border,p);}TextRenderer.DrawText(e.Graphics,Text,Font,ClientRectangle,Enabled?ForeColor:Theme.Muted,TextFormatFlags.HorizontalCenter|TextFormatFlags.VerticalCenter|TextFormatFlags.EndEllipsis);if(Focused)ControlPaint.DrawFocusRectangle(e.Graphics,new Rectangle(5,5,Width-10,Height-10));}
 }
 internal sealed class ModernCheckBox:CheckBox {
  internal ModernCheckBox(){BackColor=Theme.Surface;SetStyle(ControlStyles.UserPaint|ControlStyles.OptimizedDoubleBuffer|ControlStyles.AllPaintingInWmPaint,true);Cursor=Cursors.Hand;AutoSize=false;}
  protected override void OnPaint(PaintEventArgs e){e.Graphics.Clear(BackColor);e.Graphics.SmoothingMode=SmoothingMode.AntiAlias;int side=Math.Max(16,Math.Min(22,Height-6));var rect=new Rectangle(1,(Height-side)/2,side,side);using(var p=Theme.Shape(rect,4))using(var b=new SolidBrush(Checked?Theme.Green:Theme.Background))using(var pen=new Pen(Theme.Border)){e.Graphics.FillPath(b,p);e.Graphics.DrawPath(pen,p);}if(Checked)using(var pen=new Pen(Color.FromArgb(7,34,22),2.5f)){e.Graphics.DrawLines(pen,new[]{new Point(rect.Left+4,rect.Top+side/2),new Point(rect.Left+side/2-1,rect.Bottom-5),new Point(rect.Right-4,rect.Top+5)});}TextRenderer.DrawText(e.Graphics,Text,Font,new Rectangle(side+11,0,Math.Max(0,Width-side-11),Height),Enabled?ForeColor:Theme.Muted,TextFormatFlags.Left|TextFormatFlags.VerticalCenter|TextFormatFlags.EndEllipsis);if(Focused)ControlPaint.DrawFocusRectangle(e.Graphics,ClientRectangle);}
 }
 internal sealed class DarkComboBox:ComboBox {
  internal DarkComboBox(){DrawMode=DrawMode.OwnerDrawFixed;FlatStyle=FlatStyle.Flat;BackColor=Theme.Surface;ForeColor=Theme.Text;ItemHeight=24;}
  protected override void OnDrawItem(DrawItemEventArgs e){using(var b=new SolidBrush((e.State&DrawItemState.Selected)!=0?Color.FromArgb(25,76,56):Theme.Surface))e.Graphics.FillRectangle(b,e.Bounds);if(e.Index>=0)TextRenderer.DrawText(e.Graphics,GetItemText(Items[e.Index]),Font,e.Bounds,Theme.Text,TextFormatFlags.Left|TextFormatFlags.VerticalCenter|TextFormatFlags.EndEllipsis);if((e.State&DrawItemState.Focus)!=0)e.DrawFocusRectangle();}
 }
}
