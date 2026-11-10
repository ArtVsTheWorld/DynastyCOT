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
   string url="https://api.github.com/repos/"+module.Repository+"/releases?per_page=30";
   // The query is fixed by this client, never accepted from downloaded metadata.
   url=url.Replace("?per_page=30","");
   var docs=Json.Serializer().DeserializeObject(Encoding.UTF8.GetString(Read(module,url,4*1024*1024,true,cancel))) as object[];if(docs==null)throw new IOException("Invalid GitHub releases response.");var result=new List<ModuleRelease>();
   foreach(var raw in docs){var doc=raw as Dictionary<string,object>;try{
    if(doc==null||(bool)doc["draft"])continue;string tag=(string)doc["tag_name"];if(!tag.StartsWith("v"))continue;string version=tag.Substring(1);ReleaseRules.Numeric(version);
    var assets=(object[])doc["assets"];string name=module.Id+"-"+version+".zip",proof=module.Id+"-"+version+"-verification.json";
    var zip=assets.Cast<Dictionary<string,object>>().SingleOrDefault(x=>(string)x["name"]==name);if(zip==null||!assets.Cast<Dictionary<string,object>>().Any(x=>(string)x["name"]==proof))continue;
    object digest;if(!zip.TryGetValue("digest",out digest)||!(digest is string)||!((string)digest).StartsWith("sha256:"))continue;
    var release=new ModuleRelease{Module=module.Id,Version=version,Tag=tag,ArchiveName=name,ArchiveUrl=(string)zip["browser_download_url"],ArchiveBytes=Convert.ToInt64(zip["size"]),ArchiveSha256=((string)digest).Substring(7),ProofUrl="https://github.com/"+module.Repository+"/releases/download/"+tag+"/"+proof,PageUrl=(string)doc["html_url"],Notes=doc.ContainsKey("body")?(string)doc["body"]:"No notes supplied.",Prerelease=(bool)doc["prerelease"],Compatibility="Read the release notes for game/mode limits. Unknown native hooks remain protected. Use one provider of each feature; older mixed modules may require matching versions."};
    ReleaseRules.Validate(module,release);if(release.Notes!=null&&release.Notes.Length>100000)throw new IOException("Release notes exceed their limit.");result.Add(release);
   }catch(InvalidCastException){}catch(KeyNotFoundException){}catch(ArgumentException){}catch(IOException){} }
   return result.OrderByDescending(x=>ReleaseRules.Numeric(x.Version)).ThenBy(x=>x.Prerelease).ToList();
  }
  internal PreparedModule Prepare(Module module,ModuleRelease release,string cache,CancellationToken cancel,Action<string> progress){
   ReleaseRules.Validate(module,release);progress("Verifying "+module.Title+" "+release.Version+"...");
   var proof=Json.Serializer().DeserializeObject(Encoding.UTF8.GetString(Read(module,release.ProofUrl,2*1024*1024,false,cancel))) as Dictionary<string,object>;object version,hash,dll,owner;
   if(proof==null||!proof.TryGetValue("version",out version)||(string)version!=release.Version||!proof.TryGetValue("archiveSha256",out hash)||(string)hash!=release.ArchiveSha256||!proof.TryGetValue("dllSha256",out dll)||!(dll is string)||!((proof.TryGetValue("name",out owner)||proof.TryGetValue("module",out owner))&&owner is string&&(string)owner==module.Id))throw new IOException("Release integrity metadata belongs to a different module/version.");
   ReleaseRules.Hash((string)dll);if(release.DllSha256!=null&&release.DllSha256!=(string)dll)throw new IOException("Pinned plugin hash differs from release metadata.");release.DllSha256=(string)dll;
   Directory.CreateDirectory(cache);Paths.NoLinks(cache,cache);string archive=Paths.Resolve(cache,release.ArchiveSha256+".zip");
   if(!File.Exists(archive)||Paths.HashFile(archive)!=release.ArchiveSha256){progress("Downloading "+module.Title+" "+release.Version+" only...");var bytes=Read(module,release.ArchiveUrl,release.ArchiveBytes,false,cancel);if(bytes.LongLength!=release.ArchiveBytes||Paths.Hash(bytes)!=release.ArchiveSha256)throw new IOException("Downloaded ZIP failed size/SHA-256 checks. Nothing was installed.");cancel.ThrowIfCancellationRequested();Paths.Atomic(archive,bytes);}
   if(new FileInfo(archive).Length!=release.ArchiveBytes||Paths.HashFile(archive)!=release.ArchiveSha256)throw new IOException("Cached ZIP failed verification.");
   using(var stream=File.OpenRead(archive))using(var zip=new ZipArchive(stream,ZipArchiveMode.Read)){
    var names=new[]{module.FileName,"README.txt","FEATURES.md"};if(zip.Entries.Count!=3||!zip.Entries.Select(x=>x.FullName).OrderBy(x=>x,StringComparer.Ordinal).SequenceEqual(names.OrderBy(x=>x,StringComparer.Ordinal)))throw new IOException("Standalone ZIP must contain only its plugin, README.txt and FEATURES.md.");
    Func<string,long,byte[]> read=(name,limit)=>{var entry=zip.GetEntry(name);if(entry.Length>limit)throw new IOException("Standalone file exceeds its limit.");using(var input=entry.Open())using(var output=new MemoryStream()){byte[] chunk=new byte[65536];int n;while((n=input.Read(chunk,0,chunk.Length))>0){cancel.ThrowIfCancellationRequested();if(output.Length+n>limit)throw new IOException("Expanded file exceeds its limit.");output.Write(chunk,0,n);}return output.ToArray();}};
    var plugin=read(module.FileName,256L*1024*1024);if(Paths.Hash(plugin)!=release.DllSha256)throw new IOException("Plugin failed release SHA-256 verification.");string file=Paths.Resolve(cache,release.DllSha256+".dll");if(Paths.Existing(file)!=release.DllSha256)Paths.Atomic(file,plugin);
    var assembly=System.Reflection.AssemblyName.GetAssemblyName(file);if(assembly.Name!=module.Id||new Version(assembly.Version.Major,assembly.Version.Minor,assembly.Version.Build)!=ReleaseRules.Numeric(release.Version))throw new IOException("Standalone DLL identity/version differs from its module.");
    var prepared=new PreparedModule{Module=module,Release=release,Dll=file,Readme=read("README.txt",128*1024),Features=read("FEATURES.md",512*1024)};
    object rawPrograms;if(proof.TryGetValue("payload",out rawPrograms)){
     var programs=rawPrograms as Dictionary<string,object>;if(programs==null||programs.Count>2000)throw new IOException("Invalid release program metadata.");
     foreach(var item in programs){string path=item.Key.Replace('\\','/');Paths.Relative(path);if(!CollectionInstaller.ProgramPath(path))continue;string digest=item.Value as string;ReleaseRules.Hash(digest);prepared.Programs.Add(new KnownFile{Path=path,Module=module.Id,Sha256=digest});}
    }return prepared;
   }
  }
 }
}
