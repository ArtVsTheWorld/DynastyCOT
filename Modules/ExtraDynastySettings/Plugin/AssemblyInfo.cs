using System.Reflection;
using System.Runtime.InteropServices;
using Frosty.Core.Attributes;
[assembly: AssemblyTitle("Extra Settings")]
[assembly: AssemblyDescription("Game settings and deferred Controller Select in offline Dynasty")]
[assembly: AssemblyProduct("Extra Settings")]
[assembly: AssemblyVersion("0.2.12.0")]
[assembly: AssemblyFileVersion("0.2.12.0")]
[assembly: PluginAuthor("Ace")]
[assembly: PluginVersion("0.2.12")]
[assembly: PluginDisplayName("Ace\u2019s Extra Settings")]
[assembly: RegisterExecutionAction(typeof(OfflineDynastySettings.CombinedSettingsAction))]
[assembly: ComVisible(false)]
[assembly: System.Runtime.CompilerServices.InternalsVisibleTo("LaunchChecks")]
