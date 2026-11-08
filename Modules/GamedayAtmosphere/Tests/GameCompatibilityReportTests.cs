using System;
using System.IO;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.Serialization.Json;
using DynamicGamedayAtmosphere.Diagnostics;
class GameCompatibilityReportTests {
    static void Check(bool condition){if(!condition)throw new Exception("Compatibility report regression");}
    static int Main(){
        string folder=Path.Combine(Path.GetTempPath(),"DGA-report-"+Guid.NewGuid().ToString("N"));
        try{
            var sites=GameCompatibility.Sites("CrowdVisibility 1000\nSackPredicate 2000\nOutside 9000\n");
            Check(sites.Count==3&&sites[0].Rva==0x1000);
            foreach(string invalid in new[]{"","Site nope","Site 20","Site 1000 extra","Site 1000\nSite 2000"}){
                bool failed=false;try{GameCompatibility.Sites(invalid);}catch(InvalidDataException){failed=true;}Check(failed);
            }
            byte[] memory=new byte[0x3000];memory[0]=(byte)'M';memory[1]=(byte)'Z';BitConverter.GetBytes(0x80).CopyTo(memory,0x3c);
            BitConverter.GetBytes(0x4550).CopyTo(memory,0x80);BitConverter.GetBytes((ushort)0x8664).CopyTo(memory,0x84);BitConverter.GetBytes(12345u).CopyTo(memory,0x88);
            memory[0x1000]=0x48;memory[0x1001]=0x89;
            const ulong image=0x140000000;
            GameCompatibility.Copy read=delegate(ulong address,byte[] output){
                if(address<image||address-image>(ulong)(memory.Length-output.Length)||address==image+0x2000)return false;
                Array.Copy(memory,(int)(address-image),output,0,output.Length);return true;
            };
            var report=new GameReport{ProcessId=42,CreatedUtc="test",Executable="CollegeFB27.exe",FileVersion="1.0",ProductVersion="2027"};
            GameCompatibility.Sample(report,image,memory.Length,sites,read);
            Check(report.PeTimestamp==12345&&report.Code.Count==3&&report.Code[0].Bytes.StartsWith("48 89 ")&&report.Code[1].Error.Contains("not readable")&&report.Code[2].Error.Contains("outside"));
            Check(memory[0x1000]==0x48&&memory[0x1001]==0x89);
            bool badHeader=false;memory[0x80]=0;try{GameCompatibility.Sample(new GameReport(),image,memory.Length,sites,read);}catch(InvalidDataException){badHeader=true;}Check(badHeader);memory[0x80]=0x50;
            string path=GameCompatibility.Write(folder,report);Check(Path.GetDirectoryName(path)==Path.Combine(folder,"Diagnostics"));
            using(var file=File.OpenRead(path)){
                var saved=(GameReport)new DataContractJsonSerializer(typeof(GameReport)).ReadObject(file);
                Check(saved.Code.Count==3&&saved.ProcessId==42&&saved.PeTimestamp==12345&&saved.Executable=="CollegeFB27.exe");
            }
            Check(GameCompatibility.Write(folder,report)!=path&&Directory.GetFiles(Path.Combine(folder,"Diagnostics")).Length==2);
            // Exercise the actual Windows read ABI against THIS test's module,
            // never a game. Validate the compiled tool's address resource too.
            using(var self=Process.GetCurrentProcess()){
                var actual=new GameReport();GameCompatibility.SampleProcess(actual,self,sites);
                // Managed test EXEs may leave RVA 0x1000 unmapped between
                // headers and .text; reporting that unreadable gap is valid.
                Check(actual.ImageSize>0&&actual.PeTimestamp>0&&actual.Code.Count==3&&actual.ImageBase.StartsWith("0x"));
            }
            var tool=Assembly.ReflectionOnlyLoadFrom(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"DgaDiagnostics.exe"));
            using(var resource=tool.GetManifestResourceStream("Dga.CompatibilitySites"))using(var reader=new StreamReader(resource)){
                var embedded=GameCompatibility.Sites(reader.ReadToEnd());Check(embedded.Count==21&&embedded.Exists(x=>x.Name=="CrowdVisibility"&&x.Rva==0x64b3930));
                Check(embedded.Exists(x=>x.Name=="StartingAttendanceFormula"&&x.Rva==0x856a4b0)&&embedded.Exists(x=>x.Name=="StartingAttendanceLaunch"&&x.Rva==0x90a18b0));
                Check(embedded.Exists(x=>x.Name=="StartingAttendanceBoundary"&&x.Rva==0x7f182c8));
                Check(embedded.Exists(x=>x.Name=="ReplayNodeUpdate"&&x.Rva==0x7dc27f0)&&embedded.Exists(x=>x.Name=="ReplayEventNodeUpdate"&&x.Rva==0x7dcb0c0));
            }
            Console.WriteLine("PASS: bounded read-only samples, real Windows reads against the test itself, embedded addresses, missing/outside addresses, PE validation, JSON round-trip and nested reports. No game process opened.");return 0;
        }catch(Exception error){Console.Error.WriteLine(error);return 1;}
        finally{if(Directory.Exists(folder))Directory.Delete(folder,true);}
    }
}
