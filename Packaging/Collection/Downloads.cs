using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;

namespace DynastyCOT.Collection {
 internal sealed class Downloads {
  readonly Func<string,long,byte[]> fixtureRead;
  internal Downloads(){}
  internal Downloads(Fixture fixture,Func<string,long,byte[]> reader){if(fixture==null||reader==null)throw new IOException("Download injection is fixture-only.");fixtureRead=reader;}
  internal byte[] Read(Module module,string url,long maximum,bool api,CancellationToken cancel){
   ReleaseRules.NetworkUrl(module,url,api);cancel.ThrowIfCancellationRequested();if(fixtureRead!=null){var bytes=fixtureRead(url,maximum);if(bytes.LongLength>maximum)throw new IOException("Fixture response exceeds its limit.");return bytes;}
   ServicePointManager.SecurityProtocol|=SecurityProtocolType.Tls12;var watch=Stopwatch.StartNew();Uri at=new Uri(url);
   for(int redirects=0;redirects<8;++redirects){var request=(HttpWebRequest)WebRequest.Create(at);request.UserAgent="DynastyCOT-Collection/"+CollectionInfo.Version;request.Accept="application/vnd.github+json";request.AllowAutoRedirect=false;request.Timeout=30000;request.ReadWriteTimeout=30000;
    using(cancel.Register(()=>request.Abort()))try{using(var response=(HttpWebResponse)request.GetResponse()){
     int code=(int)response.StatusCode;if(code>=300&&code<400){Uri next;if(!Uri.TryCreate(at,response.Headers["Location"],out next)||next.Scheme!="https"||!String.IsNullOrEmpty(next.UserInfo)||!next.IsDefaultPort||!new[]{"github.com","api.github.com","release-assets.githubusercontent.com","objects.githubusercontent.com","github-releases.githubusercontent.com"}.Contains(next.Host))throw new IOException("Download redirected outside supported GitHub hosts.");at=next;continue;}
     if(code!=200||response.ContentLength>maximum)throw new IOException("Release response is unavailable or too large.");using(var input=response.GetResponseStream())using(var output=new MemoryStream()){byte[] buffer=new byte[65536];int n;while((n=input.Read(buffer,0,buffer.Length))>0){cancel.ThrowIfCancellationRequested();if(output.Length+n>maximum||watch.Elapsed.TotalMinutes>4)throw new IOException("Download exceeded its size/time limit.");output.Write(buffer,0,n);}if(response.ContentLength>=0&&output.Length!=response.ContentLength)throw new IOException("Interrupted download. Nothing was installed.");return output.ToArray();}
    }}catch(WebException e){cancel.ThrowIfCancellationRequested();throw new IOException("Could not download "+module.Title+". Check your connection or GitHub rate limits; installed files are unchanged. You can keep your current version or use its standalone ZIP.",e);}
   }throw new IOException("Too many download redirects.");
  }
  internal List<ModuleRelease> Check(Module module,CancellationToken cancel){
   var result=new List<ModuleRelease>();
   for(int page=1;page<=1000;++page){
   cancel.ThrowIfCancellationRequested();
   string url="https://api.github.com/repos/"+module.Repository+"/releases?per_page=100&page="+page;
   var docs=Json.Serializer().DeserializeObject(Encoding.UTF8.GetString(Read(module,url,8*1024*1024,true,cancel))) as object[];if(docs==null)throw new IOException("Invalid GitHub releases response.");
   foreach(var raw in docs){var doc=raw as Dictionary<string,object>;try{
    if(doc==null||(bool)doc["draft"])continue;string tag=(string)doc["tag_name"];if(!tag.StartsWith("v"))continue;string version=tag.Substring(1);ReleaseRules.Numeric(version);
    var assets=(object[])doc["assets"];string name=module.Id+"-"+version+".zip",proof=module.Id+"-"+version+"-verification.json";
    var zip=assets.Cast<Dictionary<string,object>>().SingleOrDefault(x=>(string)x["name"]==name);if(zip==null||!assets.Cast<Dictionary<string,object>>().Any(x=>(string)x["name"]==proof))continue;
    object digest;if(!zip.TryGetValue("digest",out digest)||!(digest is string)||!((string)digest).StartsWith("sha256:"))continue;
    var release=new ModuleRelease{Module=module.Id,Version=version,Tag=tag,ArchiveName=name,ArchiveUrl=(string)zip["browser_download_url"],ArchiveBytes=Convert.ToInt64(zip["size"]),ArchiveSha256=((string)digest).Substring(7),ProofUrl="https://github.com/"+module.Repository+"/releases/download/"+tag+"/"+proof,PageUrl=(string)doc["html_url"],Notes=doc.ContainsKey("body")?(string)doc["body"]:"No notes supplied.",Prerelease=(bool)doc["prerelease"],Compatibility="Read the release notes for game/mode limits. Unknown native hooks remain protected. Use one provider of each feature; older mixed modules may require matching versions."};
    ReleaseRules.Validate(module,release);if(release.Notes!=null&&release.Notes.Length>100000)throw new IOException("Release notes exceed their limit.");result.Add(release);
   }catch(InvalidCastException){}catch(KeyNotFoundException){}catch(ArgumentException){}catch(IOException){} }
   if(docs.Length<100)return result.GroupBy(x=>x.Tag,StringComparer.Ordinal).Select(x=>x.First()).OrderByDescending(x=>ReleaseRules.Numeric(x.Version)).ThenBy(x=>x.Prerelease).ToList();
   }throw new IOException("Release history exceeded the supported limit. No partial version list was used.");
  }
  // Optional offline assets beside setup retain exactly the same pinned hashes.
  byte[] Artifact(Module module,string url,string leaf,long maximum,CancellationToken cancel){
   ReleaseRules.NetworkUrl(module,url,false);cancel.ThrowIfCancellationRequested();
   string folder=AppDomain.CurrentDomain.BaseDirectory,path=Path.Combine(folder,leaf);
   if(fixtureRead==null&&File.Exists(path)){Paths.Relative(leaf);if(Path.GetFileName(leaf)!=leaf)throw new IOException("Invalid local asset name.");Paths.NoLinks(folder,path);if(new FileInfo(path).Length>maximum)throw new IOException("Local release file is too large.");return File.ReadAllBytes(path);}
   return Read(module,url,maximum,false,cancel);
  }
  internal PreparedModule Prepare(Module module,ModuleRelease release,string cache,CancellationToken cancel,Action<string> progress){
   ReleaseRules.Validate(module,release);progress("Verifying "+module.Title+" "+release.Version+"...");
   var proof=Json.Serializer().DeserializeObject(Encoding.UTF8.GetString(Artifact(module,release.ProofUrl,module.Id+"-"+release.Version+"-verification.json",2*1024*1024,cancel))) as Dictionary<string,object>;object version,hash,dll,owner;
   if(proof==null||!proof.TryGetValue("version",out version)||(string)version!=release.Version||!proof.TryGetValue("archiveSha256",out hash)||(string)hash!=release.ArchiveSha256||!proof.TryGetValue("dllSha256",out dll)||!(dll is string)||!((proof.TryGetValue("name",out owner)||proof.TryGetValue("module",out owner))&&owner is string&&(string)owner==module.Id))throw new IOException("Release integrity metadata belongs to a different module/version.");
   ReleaseRules.Hash((string)dll);if(release.DllSha256!=null&&release.DllSha256!=(string)dll)throw new IOException("Pinned plugin hash differs from release metadata.");release.DllSha256=(string)dll;
   Directory.CreateDirectory(cache);Paths.NoLinks(cache,cache);string archive=Paths.Resolve(cache,release.ArchiveSha256+".zip");
   if(!File.Exists(archive)||Paths.HashFile(archive)!=release.ArchiveSha256){progress("Downloading "+module.Title+" "+release.Version+" only...");var bytes=Artifact(module,release.ArchiveUrl,release.ArchiveName,release.ArchiveBytes,cancel);if(bytes.LongLength!=release.ArchiveBytes||Paths.Hash(bytes)!=release.ArchiveSha256)throw new IOException("Downloaded ZIP failed size/SHA-256 checks. Nothing was installed.");cancel.ThrowIfCancellationRequested();Paths.Atomic(archive,bytes);}
   if(new FileInfo(archive).Length!=release.ArchiveBytes||Paths.HashFile(archive)!=release.ArchiveSha256)throw new IOException("Cached ZIP failed verification.");
   using(var stream=File.OpenRead(archive))using(var zip=new ZipArchive(stream,ZipArchiveMode.Read)){
    var support=new Dictionary<string,string>();object supportRaw;if(proof.TryGetValue("supportFiles",out supportRaw)){var map=supportRaw as Dictionary<string,object>;if(map==null||map.Count>1)throw new IOException("Invalid support manifest.");foreach(var row in map){if(!CollectionInstaller.SupportPath(row.Key,module.Id,release.Version))throw new IOException("Unsafe support path.");string supportHash=row.Value as string;ReleaseRules.Hash(supportHash);support.Add(row.Key,supportHash);}}
    string pluginEntry=support.Count>0?"Plugins/"+module.FileName:module.FileName;
    bool hasGuide=zip.GetEntry(module.GuideName)!=null;
    bool hasLicense=zip.GetEntry("LICENSES.txt")!=null;
    object guideDigest;bool declaresGuide=proof.TryGetValue("playerGuideSha256",out guideDigest);
    if(declaresGuide){ReleaseRules.Hash(guideDigest as string);if(!hasGuide)throw new IOException("The release verification lists a player guide, but the ZIP is missing "+module.GuideName+".");}
    if(hasGuide&&(hasLicense||support.Count!=0))throw new IOException("A guide release must contain exactly its plugin, README.txt, FEATURES.md and "+module.GuideName+".");
    var names=new List<string>{pluginEntry,"README.txt","FEATURES.md"};names.AddRange(support.Keys);
    if(hasGuide)names.Add(module.GuideName);if(hasLicense)names.Add("LICENSES.txt");
    if(!zip.Entries.Select(x=>x.FullName).OrderBy(x=>x,StringComparer.Ordinal).SequenceEqual(names.OrderBy(x=>x,StringComparer.Ordinal)))throw new IOException("Standalone ZIP contains missing, repeated or unexpected files.");
    Func<string,long,byte[]> read=(name,limit)=>{var entry=zip.GetEntry(name);if(entry.Length>limit)throw new IOException("Standalone file exceeds its limit: "+name);using(var input=entry.Open())using(var output=new MemoryStream()){byte[] chunk=new byte[65536];int n;while((n=input.Read(chunk,0,chunk.Length))>0){cancel.ThrowIfCancellationRequested();if(output.Length+n>limit)throw new IOException("Expanded file exceeds its limit: "+name);output.Write(chunk,0,n);}if(output.Length!=entry.Length)throw new IOException("Incomplete standalone file: "+name);return output.ToArray();}};
    if(zip.GetEntry("LICENSES.txt")!=null)read("LICENSES.txt",1024*1024);
    var plugin=read(pluginEntry,256L*1024*1024);if(Paths.Hash(plugin)!=release.DllSha256)throw new IOException("Plugin failed release SHA-256 verification.");string file=Paths.Resolve(cache,release.DllSha256+".dll");if(Paths.Existing(file)!=release.DllSha256)Paths.Atomic(file,plugin);
    var assembly=System.Reflection.AssemblyName.GetAssemblyName(file);if(assembly.Name!=module.Id||new Version(assembly.Version.Major,assembly.Version.Minor,assembly.Version.Build)!=ReleaseRules.Numeric(release.Version))throw new IOException("Standalone DLL identity/version differs from its module.");
    var prepared=new PreparedModule{Module=module,Release=release,Dll=file,Readme=read("README.txt",module.Id=="BeyondTheField"&&hasGuide?512*1024:128*1024),Features=read("FEATURES.md",512*1024)};
    if(hasGuide){
     var guide=read(module.GuideName,10L*1024*1024);
     if(guide.Length<10||Encoding.ASCII.GetString(guide,0,5)!="%PDF-"||!Encoding.ASCII.GetString(guide,Math.Max(0,guide.Length-1024),Math.Min(1024,guide.Length)).TrimEnd(' ','\t','\r','\n','\f').EndsWith("%%EOF",StringComparison.Ordinal))throw new IOException("The player guide has an invalid PDF header or ending: "+module.GuideName);
     if(declaresGuide&&Paths.Hash(guide)!=(string)guideDigest)throw new IOException("Player guide failed release SHA-256 verification.");
     prepared.Guide=guide;
    }
    foreach(var item in support){var bytes=read(item.Key,64L*1024*1024);if(Paths.Hash(bytes)!=item.Value)throw new IOException("Native support SHA-256 differs.");prepared.SupportFiles.Add(item.Key,bytes);prepared.Programs.Add(new KnownFile{Path=item.Key,Module=module.Id,Sha256=item.Value});}
    object rawPrograms;if(proof.TryGetValue("payload",out rawPrograms)){
     var programs=rawPrograms as Dictionary<string,object>;if(programs==null||programs.Count>2000)throw new IOException("Invalid release program metadata.");
     foreach(var item in programs){string path=item.Key.Replace('\\','/');Paths.Relative(path);if(!CollectionInstaller.ProgramPath(path))continue;string digest=item.Value as string;ReleaseRules.Hash(digest);prepared.Programs.Add(new KnownFile{Path=path,Module=module.Id,Sha256=digest});}
    }return prepared;
   }
  }
 }
}
