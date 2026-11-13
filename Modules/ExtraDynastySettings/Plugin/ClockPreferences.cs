using System;
using System.Globalization;
using System.IO;
namespace OfflineDynastySettings
{
    internal static class SettingsFiles
    {
        private static readonly object sync=new object();
        internal static readonly string[] Names={"DynastyPlayClock.ini","OfflineDynastySettings.ini",
            "OfflineDynastySettings.log","OfflineDynastySettings.log.1","OfflineDynastySettings.Persistence.log",
            "DynastyPlayClock.log","OfflinePlaycall.log"};
        internal static string Folder(string pluginFolder)
        {
            string directory=Ace.DynastyCOT.UserStorage.Folder("ExtraSettings");
            Directory.CreateDirectory(directory);return directory;
        }
        internal static string PathFor(string pluginFolder,string name)
        {
            if(Array.IndexOf(Names,name)<0)throw new ArgumentException("Unknown plugin settings/log file.","name");
            lock(sync){return Path.Combine(Folder(pluginFolder),name);}
        }
        internal static void Migrate(string pluginFolder){Ace.DynastyCOT.UserStorage.Migrate("ExtraSettings",pluginFolder);}
    }
    internal static class ClockPreferences
    {
        internal const string FileName="OfflineDynastySettings.ini";
        internal static bool Valid(int seconds) { return seconds==0 || seconds==3 || seconds==5 || seconds==15 || seconds==20 || seconds==30 || seconds==45 || seconds==60; }
        internal static int Load(string folder)
        {
            string path=SettingsFiles.PathFor(folder,FileName);
            if(!File.Exists(path))return 15;
            string text=File.ReadAllText(path).Trim();int seconds;
            const string prefix="DefensivePlayClockSeconds=";
            if(!text.StartsWith(prefix,StringComparison.Ordinal) || !Int32.TryParse(text.Substring(prefix.Length),NumberStyles.None,CultureInfo.InvariantCulture,out seconds) || !Valid(seconds))
                throw new InvalidDataException("Unreadable Defensive Play Clock preference; saved file preserved.");
            return seconds;
        }
        internal static void Save(string folder,int seconds)
        {
            if(!Valid(seconds))throw new InvalidDataException("Invalid Defensive Play Clock choice.");
            string path=SettingsFiles.PathFor(folder,FileName),temporary=path+".tmp";
            using(FileStream file=new FileStream(temporary,FileMode.Create,FileAccess.Write,FileShare.None))
            using(StreamWriter writer=new StreamWriter(file)) { writer.Write("DefensivePlayClockSeconds="+seconds.ToString(CultureInfo.InvariantCulture)+"\r\n");writer.Flush();file.Flush(true); }
            if(File.Exists(path))File.Replace(temporary,path,null);else File.Move(temporary,path);
        }
    }
}
