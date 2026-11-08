using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;

namespace DynamicGamedayAtmosphere.Diagnostics {
    [DataContract] sealed class CodeSample {
        [DataMember(Order=1)] public string Name;
        [DataMember(Order=2)] public string Rva;
        [DataMember(Order=3)] public string Bytes;
        [DataMember(Order=4)] public string Error;
    }
    [DataContract] sealed class GameReport {
        [DataMember(Order=1)] public string Format="DGA-read-only-game-report-1";
        [DataMember(Order=2)] public string CreatedUtc;
        [DataMember(Order=3)] public int ProcessId;
        [DataMember(Order=4)] public string Executable;
        [DataMember(Order=5)] public string FileVersion;
        [DataMember(Order=6)] public string ProductVersion;
        [DataMember(Order=7)] public string ImageBase;
        [DataMember(Order=8)] public int ImageSize;
        [DataMember(Order=9)] public uint PeTimestamp;
        [DataMember(Order=10)] public List<string> LoadedModules=new List<string>();
        [DataMember(Order=11)] public List<CodeSample> Code=new List<CodeSample>();
        [DataMember(Order=12)] public string ModuleError;
        [DataMember(Order=13)] public string Limitation="Reads the running game's PE header and small code samples at DGA's existing addresses. Does not run game functions, change memory, find new addresses or certify compatibility. No game executable, save, full memory dump or user-folder paths are included.";
    }
    sealed class ReportSite {public string Name;public uint Rva;}
    static class GameCompatibility {
        internal delegate bool Copy(ulong address,byte[] output);
        [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr OpenProcess(uint access,bool inherit,int pid);
        [DllImport("kernel32.dll",SetLastError=true)] static extern bool ReadProcessMemory(IntPtr process,UIntPtr address,byte[] output,UIntPtr length,out UIntPtr read);
        [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);

        internal static List<ReportSite> Sites(string text){
            var result=new List<ReportSite>();var names=new HashSet<string>(StringComparer.Ordinal);
            using(var lines=new StringReader(text))for(string line;(line=lines.ReadLine())!=null;){
                if(String.IsNullOrWhiteSpace(line))continue;
                string[] parts=line.Split(new[]{' ','\t'},StringSplitOptions.RemoveEmptyEntries);uint rva;
                if(parts.Length!=2||!names.Add(parts[0])||parts[0].Length>64||!UInt32.TryParse(parts[1],NumberStyles.HexNumber,CultureInfo.InvariantCulture,out rva)||rva<0x1000||rva>0x40000000||result.Count>=64)
                    throw new InvalidDataException("Invalid embedded compatibility addresses.");
                result.Add(new ReportSite{Name=parts[0],Rva=rva});
            }
            if(result.Count==0)throw new InvalidDataException("No compatibility addresses were embedded.");
            return result;
        }
        internal static void Sample(GameReport report,ulong image,int size,List<ReportSite> sites,Copy copy){
            if(image<0x10000||size<0x1000||size>0x40000000||image>UInt64.MaxValue-(uint)size)throw new InvalidDataException("Invalid game module bounds.");
            byte[] dos=new byte[64];
            if(!copy(image,dos)||dos[0]!='M'||dos[1]!='Z')throw new InvalidDataException("The running game's PE header is unavailable.");
            uint offset=BitConverter.ToUInt32(dos,0x3c);byte[] pe=new byte[24];
            if(offset>4096||offset>(uint)size-pe.Length||!copy(image+offset,pe)||BitConverter.ToUInt32(pe,0)!=0x4550||BitConverter.ToUInt16(pe,4)!=0x8664)
                throw new InvalidDataException("The running game has an unexpected PE header.");
            report.ImageBase="0x"+image.ToString("x",CultureInfo.InvariantCulture);report.ImageSize=size;report.PeTimestamp=BitConverter.ToUInt32(pe,8);
            foreach(var site in sites){
                var sample=new CodeSample{Name=site.Name,Rva="0x"+site.Rva.ToString("x",CultureInfo.InvariantCulture)};
                byte[] bytes=new byte[32];
                if(site.Rva>(uint)size-bytes.Length)sample.Error="Address is outside this module; layout differs.";
                else if(!copy(image+site.Rva,bytes))sample.Error="Code is not readable at this address.";
                else sample.Bytes=BitConverter.ToString(bytes).Replace('-',' ').ToLowerInvariant();
                report.Code.Add(sample);
            }
        }
        internal static string Write(string data,GameReport report){
            string directory=Path.Combine(data,"Diagnostics");Directory.CreateDirectory(directory);
            string destination=Path.Combine(directory,"game-report-"+DateTime.UtcNow.ToString("yyyyMMdd-HHmmss",CultureInfo.InvariantCulture)+"-"+Guid.NewGuid().ToString("N")+".json");
            using(var file=new FileStream(destination,FileMode.CreateNew,FileAccess.Write,FileShare.Read))new DataContractJsonSerializer(typeof(GameReport)).WriteObject(file,report);
            return destination;
        }
        internal static void SampleProcess(GameReport report,Process process,List<ReportSite> sites){
            var module=process.MainModule;if(module==null)throw new IOException("The running game's module is unavailable.");
            IntPtr handle=OpenProcess(0x1010,false,process.Id); // QUERY_LIMITED_INFORMATION | VM_READ only.
            if(handle==IntPtr.Zero)throw new IOException("Windows did not allow the read-only game report (error "+Marshal.GetLastWin32Error()+").");
            try{
                Sample(report,unchecked((ulong)module.BaseAddress.ToInt64()),module.ModuleMemorySize,sites,delegate(ulong address,byte[] output){
                    UIntPtr count;return ReadProcessMemory(handle,new UIntPtr(address),output,new UIntPtr((uint)output.Length),out count)&&count.ToUInt64()==(uint)output.Length;
                });
            }finally{CloseHandle(handle);}
        }
        internal static string Capture(string data){
            Process[] games=Process.GetProcessesByName("CollegeFB27");
            try{
                if(games.Length!=1)throw new InvalidOperationException("Open exactly one College Football 27 game before saving its report.");
                using(var process=games[0]){
                    var module=process.MainModule;if(module==null)throw new IOException("The running game's module is unavailable.");
                    var report=new GameReport{CreatedUtc=DateTime.UtcNow.ToString("o",CultureInfo.InvariantCulture),ProcessId=process.Id,Executable=Path.GetFileName(module.FileName)};
                    var version=FileVersionInfo.GetVersionInfo(module.FileName);report.FileVersion=version.FileVersion;report.ProductVersion=version.ProductVersion;
                    // Names only: no paths or contents from other plugins.
                    try{foreach(ProcessModule loaded in process.Modules){if(report.LoadedModules.Count>=512)break;report.LoadedModules.Add(loaded.ModuleName);}report.LoadedModules.Sort(StringComparer.OrdinalIgnoreCase);}
                    catch(Exception error){report.ModuleError=error.GetType().Name+": module names could not all be read.";}
                    List<ReportSite> sites;
                    using(var resource=Assembly.GetExecutingAssembly().GetManifestResourceStream("Dga.CompatibilitySites")){
                        if(resource==null)throw new InvalidDataException("Compatibility addresses are missing; rebuild the diagnostics tool.");
                        using(var reader=new StreamReader(resource,Encoding.ASCII))sites=Sites(reader.ReadToEnd());
                    }
                    SampleProcess(report,process,sites);
                    return Write(data,report);
                }
            }finally{foreach(var game in games)game.Dispose();}
        }
    }
}
