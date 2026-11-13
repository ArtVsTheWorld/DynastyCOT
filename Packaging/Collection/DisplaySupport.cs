using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Windows.Forms;
namespace DynastyCOT.Collection {
 internal static class DpiSupport {
  internal static void Initialize(){
   string config=Application.ExecutablePath+".config";
   if(!System.IO.File.Exists(config)||Paths.HashFile(config)!=CollectionInfo.ConfigSha256)throw new System.IO.IOException("The installer's display configuration is missing or changed. Extract the complete setup ZIP and keep its .exe.config file beside the .exe. No installed plugins were changed.");
  }
 }
 // A bounded Markdown subset rendered as native text. No browser, HTML, remote
 // images, scripts or RTF supplied by release authors are ever executed.
 internal sealed class ReleaseNotesView:RichTextBox {
  struct Link {internal int Start,Length;internal string Url;}
  readonly List<Link> links=new List<Link>();
  readonly List<Font> fonts=new List<Font>();readonly Dictionary<string,Font> fontCache=new Dictionary<string,Font>();
  internal ReleaseNotesView(){ReadOnly=true;BorderStyle=BorderStyle.None;BackColor=Setup.Theme.Surface;ForeColor=Setup.Theme.Text;DetectUrls=false;WordWrap=true;ScrollBars=RichTextBoxScrollBars.Vertical;AccessibleName="Release notes";MouseUp+=(s,e)=>{if(e.Button!=MouseButtons.Left)return;int at=GetCharIndexFromPosition(e.Location);foreach(var link in links)if(at>=link.Start&&at<link.Start+link.Length){Process.Start(new ProcessStartInfo(link.Url){UseShellExecute=true});break;}};}
  Font Face(float size,FontStyle style,bool code=false){string key=size+"/"+style+"/"+code;Font found;if(fontCache.TryGetValue(key,out found))return found;var font=new Font(code?"Consolas":Font.FontFamily.Name,size,style);fonts.Add(font);fontCache[key]=font;return font;}
  void Add(string value,float size,FontStyle style,bool code=false){SelectionFont=Face(size,style,code);SelectionColor=ForeColor;AppendText(value);}
  void Inline(string value,float size,FontStyle initial){
   var tokens=Regex.Matches(value,@"!?\[[^\]\r\n]{1,300}\]\([^\s)]{1,2048}\)|`[^`\r\n]+`|\*\*[^*\r\n]+\*\*|(?<!\*)\*[^*\r\n]+\*(?!\*)");int pos=0;
   foreach(Match token in tokens){Add(value.Substring(pos,token.Index-pos),size,initial);string t=token.Value;
    if(t.StartsWith("![")){Add("[Image: "+t.Substring(2,t.IndexOf(']')-2)+"]",size,initial);}
    else if(t.StartsWith("[")){int split=t.IndexOf("](");string label=t.Substring(1,split-1),url=t.Substring(split+2,t.Length-split-3);Uri uri;bool safe=Uri.TryCreate(url,UriKind.Absolute,out uri)&&(uri.Scheme=="https"||uri.Scheme=="http")&&String.IsNullOrEmpty(uri.UserInfo);
     int start=TextLength;Add(label,size,safe?initial|FontStyle.Underline:initial);if(safe){Select(start,TextLength-start);SelectionColor=Color.LightSkyBlue;Select(TextLength,0);links.Add(new Link{Start=start,Length=label.Length,Url=uri.AbsoluteUri});}
    }else if(t[0]=='`')Add(t.Substring(1,t.Length-2),size,initial,true);
    else if(t.StartsWith("**"))Add(t.Substring(2,t.Length-4),size,initial|FontStyle.Bold);
    else Add(t.Substring(1,t.Length-2),size,initial|FontStyle.Italic);
    pos=token.Index+token.Length;
   }Add(value.Substring(pos),size,initial);
  }
  internal void Render(string markdown){
   Clear();links.Clear();foreach(var font in fonts)font.Dispose();fonts.Clear();fontCache.Clear();
   string value=(markdown??"").Replace("\r\n","\n").Replace('\r','\n');if(value.Length>100000)value=value.Substring(0,100000)+"\n[Notes shortened]";
   bool code=false;foreach(string raw in value.Split('\n')){string line=raw;if(line.TrimStart().StartsWith("```")){code=!code;continue;}float size=Font.SizeInPoints;FontStyle style=FontStyle.Regular;
    if(code)Add(line,size,style,true);else {var heading=Regex.Match(line,@"^ {0,3}(#{1,6})\s+(.+)$");if(heading.Success){size+=Math.Max(1,5-heading.Groups[1].Length);style=FontStyle.Bold;line=heading.Groups[2].Value;}else if(Regex.IsMatch(line,@"^\s*[-*+]\s+"))line=Regex.Replace(line,@"^\s*[-*+]\s+","• ");else if(line.StartsWith("> "))line="  "+line.Substring(2);Inline(line,size,style);}AppendText("\n");
   }Select(0,0);ScrollToCaret();
  }
  protected override void Dispose(bool disposing){if(disposing)foreach(var font in fonts)font.Dispose();base.Dispose(disposing);}
 }
}
