using System;using System.IO;
class MigrateSettings {
    static int Main(string[] args){try{
        if(args.Length==2)DynamicGamedayAtmosphere.PluginStorage.Ensure(args[0],File.ReadAllText(args[1]));
        else if(args.Length==3)DynamicGamedayAtmosphere.SettingsMigration.Ensure(args[0],args[1],File.ReadAllText(args[2]));
        else throw new ArgumentException("Usage: MigrateSettings plugins-folder defaults.ini");return 0;
    }catch(Exception e){Console.Error.WriteLine(e.Message);return 1;}}
}
