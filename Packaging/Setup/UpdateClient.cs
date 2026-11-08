using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Web.Script.Serialization;

namespace DynastyCOT.Setup {
 internal sealed class ReleaseChoice {
  internal string Version,Name,Notes,PageUrl,MetadataUrl;internal DateTime Published;internal bool Prerelease;
  public override string ToString(){return Version+(Prerelease?"  ·  test release":"");}
 }
 internal sealed class UpdateMetadata {
  internal string Version,Notes,Compatibility,ArchiveName,ArchiveUrl,ArchiveSha256;internal long ArchiveBytes;
 }
 internal sealed class VerifiedUpdate {
  internal readonly UpdateMetadata Metadata;internal readonly string Archive;
  internal VerifiedUpdate(UpdateMetadata metadata,string archive){Metadata=metadata;Archive=archive;}
 }
 // No timer, startup request, account token or background update service. Each
 // public method corresponds to a separate explicit action in the setup UI.
 internal sealed class UpdateClient {
  private readonly string repository;private readonly Uri fixtureBase;
  internal const long MaximumArchiveBytes=512L*1024*1024;
  internal UpdateClient(string repo){
   if(!Regex.IsMatch(repo??"",@"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$"))throw new IOException("This setup has no valid update repository. Use a versioned installer from the project page.");repository=repo;
  }
  internal UpdateClient(string repo,Uri testEndpoint,Fixture fixture):this(repo){
   if(fixture==null||testEndpoint==null||!testEndpoint.IsLoopback||testEndpoint.Scheme!="http")throw new IOException("Update tests require a local fixture server.");fixtureBase=testEndpoint;
  }
  internal static string VersionText(object value){
   string text=value as string;if(text==null||!Regex.IsMatch(text,@"^\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?$"))throw new IOException("The release has an invalid version.");return text;
  }
  internal static string StringField(Dictionary<string,object> doc,string key,int limit){object value;if(doc==null||!doc.TryGetValue(key,out value)||!(value is string)||((string)value).Length>limit)throw new IOException("Invalid release field: "+key);return (string)value;}
  private Uri ValidateUrl(string value,bool api){
   Uri url;if(!Uri.TryCreate(value,UriKind.Absolute,out url)||!String.IsNullOrEmpty(url.UserInfo))throw new IOException("Invalid release download address.");
   if(fixtureBase!=null){if(url.Scheme!=fixtureBase.Scheme||url.Host!=fixtureBase.Host||url.Port!=fixtureBase.Port)throw new IOException("Fixture update address left its local server.");return url;}
   if(url.Scheme!="https")throw new IOException("Release downloads require HTTPS.");
   bool primary=String.Equals(url.Host,api?"api.github.com":"github.com",StringComparison.OrdinalIgnoreCase);
   if(!primary)throw new IOException("Release address is outside the configured GitHub project.");
   string prefix=api?"/repos/"+repository+"/":"/"+repository+"/releases/";
   if(!url.AbsolutePath.StartsWith(prefix,StringComparison.OrdinalIgnoreCase))throw new IOException("Release address belongs to another project.");return url;
  }
  private bool AllowedRedirect(Uri url){
   if(fixtureBase!=null)return url.Scheme==fixtureBase.Scheme&&url.Host==fixtureBase.Host&&url.Port==fixtureBase.Port;
   if(url.Scheme!="https"||!String.IsNullOrEmpty(url.UserInfo))return false;
   foreach(string host in new[]{"github.com","api.github.com","release-assets.githubusercontent.com","objects.githubusercontent.com","github-releases.githubusercontent.com"})if(String.Equals(url.Host,host,StringComparison.OrdinalIgnoreCase))return true;
   return false;
  }
  private void Read(Uri url,long maximum,Stream output,Action<long,long> progress){
   ServicePointManager.SecurityProtocol|=SecurityProtocolType.Tls12;
   var elapsed=Stopwatch.StartNew();
   for(int redirects=0;redirects<8;redirects++){
    var request=(HttpWebRequest)WebRequest.Create(url);request.UserAgent="DynastyCOT-Setup/"+SetupInfo.Version;request.Accept="application/vnd.github+json";request.Timeout=30000;request.ReadWriteTimeout=30000;request.AllowAutoRedirect=false;
    HttpWebResponse response;
    try{response=(HttpWebResponse)request.GetResponse();}catch(WebException e){
     var failure=e.Response as HttpWebResponse;int code=failure==null?0:(int)failure.StatusCode;if(failure!=null)failure.Dispose();
     if(code==404||code==401||code==403)throw new IOException("GitHub releases are unavailable. The project may be private, access-limited, or rate-limited. Setup contains no GitHub credentials. You can keep this version or use a manually downloaded installer.");
     throw new IOException("Could not contact the release server. Your installation has not changed.",e);
    }
    using(response){
     int code=(int)response.StatusCode;
     if(code>=300&&code<=399){Uri next;if(!Uri.TryCreate(url,response.Headers["Location"],out next)||!AllowedRedirect(next))throw new IOException("Release download redirected outside the supported hosts.");url=next;continue;}
     if(code!=200||response.ContentLength>maximum)throw new IOException("Release download is unavailable or exceeds its size limit.");
     using(var input=response.GetResponseStream()){
      byte[] buffer=new byte[65536];long received=0;int n;
      while((n=input.Read(buffer,0,buffer.Length))>0){received+=n;if(received>maximum)throw new IOException("Release download exceeds its size limit.");if(elapsed.Elapsed.TotalMinutes>4)throw new IOException("Release download timed out. Nothing was installed.");output.Write(buffer,0,n);if(progress!=null)progress(received,response.ContentLength);}
      if(response.ContentLength>=0&&received!=response.ContentLength)throw new IOException("Release download was interrupted.");
     }return;
    }
   }throw new IOException("Too many release download redirects.");
  }
  private object ReadJson(Uri url,int maximum){using(var bytes=new MemoryStream()){Read(url,maximum,bytes,null);return new JavaScriptSerializer{MaxJsonLength=maximum,RecursionLimit=40}.DeserializeObject(Encoding.UTF8.GetString(bytes.ToArray()));}}
  internal List<ReleaseChoice> Check(){
   Uri endpoint=fixtureBase??new Uri("https://api.github.com/repos/"+repository+"/releases?per_page=50");
   var rows=ReadJson(endpoint,4*1024*1024) as object[];if(rows==null)throw new IOException("GitHub returned an invalid release list.");
   var choices=new List<ReleaseChoice>();
   foreach(object item in rows){
    var row=item as Dictionary<string,object>;object value;if(row==null||!row.TryGetValue("draft",out value)||!(value is bool)||(bool)value)continue;
    string tag=StringField(row,"tag_name",100);if(tag.StartsWith("v",StringComparison.Ordinal))tag=tag.Substring(1);VersionText(tag);
    if(!row.TryGetValue("assets",out value)||!(value is object[]))continue;string metadata=null;
    foreach(object asset in (object[])value){var entry=asset as Dictionary<string,object>;if(entry==null)continue;object name;if(entry.TryGetValue("name",out name)&&(string)name=="cot-update.json")metadata=ValidateUrl(StringField(entry,"browser_download_url",2048),false).AbsoluteUri;}
    if(metadata==null)continue;
    DateTime published;DateTime.TryParse(StringField(row,"published_at",100),out published);
    choices.Add(new ReleaseChoice{Version=tag,Name=StringField(row,"name",500),Notes=StringField(row,"body",65536),PageUrl=ValidateUrl(StringField(row,"html_url",2048),false).AbsoluteUri,MetadataUrl=metadata,Published=published,Prerelease=row.TryGetValue("prerelease",out value)&&value is bool&&(bool)value});
   }
   choices.Sort((a,b)=>b.Published.CompareTo(a.Published));return choices;
  }
  internal UpdateMetadata Inspect(ReleaseChoice release){
   var doc=ReadJson(ValidateUrl(release.MetadataUrl,false),1024*1024) as Dictionary<string,object>;object field;
   if(doc==null||!doc.TryGetValue("format",out field)||!(field is int)||(int)field!=1||StringField(doc,"product",100)!="DynastyCOT")throw new IOException("Invalid COT release metadata.");
   string version=VersionText(StringField(doc,"version",100));if(version!=release.Version)throw new IOException("Release metadata version does not match the selected release.");
   var archive=doc.ContainsKey("archive")?doc["archive"] as Dictionary<string,object>:null;
   string name=StringField(archive,"name",200),hash=StringField(archive,"sha256",64),url=StringField(archive,"url",2048);
   if(name!="DynastyCOT-"+version+".zip"||!Regex.IsMatch(hash,@"^[a-f0-9]{64}$"))throw new IOException("Invalid release archive identity or checksum.");
   long size;if(!archive.TryGetValue("bytes",out field)||!Int64.TryParse(Convert.ToString(field),out size)||size<1||size>MaximumArchiveBytes)throw new IOException("Invalid release archive size.");
   var location=ValidateUrl(url,false);if(!location.AbsolutePath.EndsWith("/"+name,StringComparison.Ordinal))throw new IOException("Release archive filename does not match its metadata.");
   string notes=StringField(doc,"releaseNotes",65536),compatibility=StringField(doc,"compatibilityNotes",32768);
   foreach(var pair in new[]{new[]{"releaseNotesSha256",notes},new[]{"compatibilityNotesSha256",compatibility}}){string expected=StringField(doc,pair[0],64);using(var bytes=new MemoryStream(Encoding.UTF8.GetBytes(pair[1])))if(!Regex.IsMatch(expected,@"^[a-f0-9]{64}$")||InstallerEngine.Hash(bytes)!=expected)throw new IOException("Release notes or compatibility information failed verification.");}
   return new UpdateMetadata{Version=version,ArchiveName=name,ArchiveSha256=hash,ArchiveUrl=location.AbsoluteUri,ArchiveBytes=size,Notes=notes,Compatibility=compatibility};
  }
  internal VerifiedUpdate Download(UpdateMetadata metadata,string destination,Action<long,long> progress){
   if(metadata==null)throw new IOException("Choose and inspect a release first.");
   Directory.CreateDirectory(destination);string archive=Path.Combine(destination,metadata.ArchiveName),temporary=archive+"."+Guid.NewGuid().ToString("N")+".part";
   try{
    using(var output=new FileStream(temporary,FileMode.CreateNew,FileAccess.Write))Read(ValidateUrl(metadata.ArchiveUrl,false),metadata.ArchiveBytes,output,progress);
    string hash;using(var input=File.OpenRead(temporary))hash=InstallerEngine.Hash(input);
    if(new FileInfo(temporary).Length!=metadata.ArchiveBytes||hash!=metadata.ArchiveSha256)throw new IOException("Downloaded package did not match its size and SHA-256. Nothing was installed.");
    if(File.Exists(archive))File.Delete(archive);File.Move(temporary,archive);return new VerifiedUpdate(metadata,archive);
   }finally{if(File.Exists(temporary))File.Delete(temporary);}
  }
 }
}
