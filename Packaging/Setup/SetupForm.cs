using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace DynastyCOT.Setup {
 internal sealed class SetupForm : Form {
  private static readonly Color Ink=Theme.Text,Muted=Theme.Muted,Green=Theme.Green,Canvas=Theme.Background,Line=Theme.Border;
  private readonly InstallerEngine engine=new InstallerEngine();private readonly Fixture fixture;
  private readonly TextBox target=new TextBox(),details=new TextBox(),releaseInfo=new TextBox();
  private readonly CheckBox[] modules=new CheckBox[4];
  private readonly Label[] moduleDescriptions=new Label[4];
  private readonly Label hint=new Label(),status=new Label(),installed=new Label(),selectionHint=new Label(),packageSource=new Label();
  private readonly Button browse=new RoundedButton(),install=new RoundedButton(),restore=new RoundedButton(),close=new RoundedButton(),copy=new RoundedButton(),checkUpdates=new RoundedButton(),download=new RoundedButton(),bundled=new RoundedButton();
  private readonly ComboBox backups=new DarkComboBox(),operation=new DarkComboBox(),releases=new DarkComboBox();
  private readonly ProgressBar progress=new ProgressBar();private readonly Panel settings=new Panel(),navigation=new Panel();
  private readonly Dictionary<string,Panel> pages=new Dictionary<string,Panel>();private readonly Dictionary<string,Button> navButtons=new Dictionary<string,Button>();
  private readonly BackgroundWorker worker=new BackgroundWorker(),network=new BackgroundWorker();
  private string loadedRoot,rootError="Choose your MMC Mod Manager folder.",runningAction="Install",networkAction;
  private Rectangle? layoutArea;private Rectangle lastWorkingArea;
  private string currentPage="Install";
  private bool busy,restoring,loadingReleases,fitting;private UpdateMetadata inspected;private VerifiedUpdate selectedUpdate;
  internal Func<List<StandaloneConflict>,bool> ReviewStandalone;
  private UpdateClient updates;private string downloads;
  internal bool Busy{get{return busy;}}internal bool LastSuccess{get;private set;}internal string StatusText{get{return status.Text;}}internal string DetailsText{get{return details.Text;}}
  internal SetupForm(Fixture testFixture){
   fixture=testFixture;Text="DYNASTY COT by Ace"+(fixture==null?"":" (TEST FIXTURE)");
   Font=new Font("Segoe UI",10);BackColor=Canvas;ForeColor=Ink;ClientSize=new Size(1020,640);MinimumSize=new Size(900,600);AutoScaleMode=AutoScaleMode.Dpi;StartPosition=FormStartPosition.CenterScreen;
   using(var resource=typeof(SetupForm).Assembly.GetManifestResourceStream("DynastyCOT.Crown.ico")){if(resource!=null)using(var icon=new Icon(resource))Icon=(Icon)icon.Clone();}
   var side=new Panel{Dock=DockStyle.Left,Width=156,BackColor=Color.FromArgb(10,17,21)};Controls.Add(side);
   side.Controls.Add(LabelAt("DYNASTY\nCOT",20,22,126,70,15,FontStyle.Bold,Ink));
   side.Controls.Add(LabelAt("by Ace",21,100,112,25,10,FontStyle.Regular,Muted));navigation.SetBounds(12,157,132,216);side.Controls.Add(navigation);
   string[] labels={"Install","Updates","Backups","Activity"},captions={"Install","Updates","Undo changes","Details"};for(int i=0;i<labels.Length;i++){string label=labels[i];var button=new RoundedButton{Text=captions[i],TextAlign=ContentAlignment.MiddleLeft,BackColor=side.BackColor,ForeColor=Ink,AccessibleName=captions[i]};button.SetBounds(0,i*49,132,42);button.Click+=(sender,e)=>ShowPage(label);navigation.Controls.Add(button);navButtons.Add(label,button);}
   side.Controls.Add(new Label{Text=SetupInfo.Version,ForeColor=Muted,Font=new Font(Font.FontFamily,9),Dock=DockStyle.Bottom,Height=43,Padding=new Padding(20,8,0,0)});
   var main=new TableLayoutPanel{Dock=DockStyle.Fill,Padding=new Padding(22,18,22,14),ColumnCount=1,RowCount=5};main.RowStyles.Add(new RowStyle(SizeType.Absolute,106));main.RowStyles.Add(new RowStyle(SizeType.Percent,100));main.RowStyles.Add(new RowStyle(SizeType.Absolute,34));main.RowStyles.Add(new RowStyle(SizeType.Absolute,6));main.RowStyles.Add(new RowStyle(SizeType.AutoSize));Controls.Add(main);main.BringToFront();
   var destinationCard=new RoundedPanel{Dock=DockStyle.Fill,Padding=new Padding(14,9,14,8)};main.Controls.Add(destinationCard,0,0);
   var destination=new TableLayoutPanel{Dock=DockStyle.Fill,BackColor=Theme.Surface,ColumnCount=2,RowCount=3};destination.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));destination.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute,112));destination.RowStyles.Add(new RowStyle(SizeType.Absolute,25));destination.RowStyles.Add(new RowStyle(SizeType.Absolute,34));destination.RowStyles.Add(new RowStyle(SizeType.Percent,100));destinationCard.Controls.Add(destination);
   destination.Controls.Add(new Label{Text="Mod Manager",AutoSize=true,Font=new Font(Font.FontFamily,12,FontStyle.Bold),ForeColor=Ink},0,0);installed.AutoEllipsis=true;installed.Dock=DockStyle.Fill;installed.TextAlign=ContentAlignment.TopRight;installed.Font=new Font(Font.FontFamily,8);destination.Controls.Add(installed,1,0);
   var pathBox=new RoundedPanel{Dock=DockStyle.Fill,Padding=new Padding(9,6,9,4),Margin=new Padding(0,0,8,0)};target.BorderStyle=BorderStyle.None;target.BackColor=Theme.Surface;target.ForeColor=Ink;target.Dock=DockStyle.Fill;target.AccessibleName="Mod Manager location";pathBox.Controls.Add(target);destination.Controls.Add(pathBox,0,1);StyleButton(browse,"Find exe…",false);browse.Dock=DockStyle.Fill;destination.Controls.Add(browse,1,1);hint.Dock=DockStyle.Fill;hint.ForeColor=Muted;hint.Font=new Font(Font.FontFamily,9);destination.Controls.Add(hint,0,2);destination.SetColumnSpan(hint,2);
   var pageHost=new Panel{Dock=DockStyle.Fill,Padding=new Padding(0,14,0,8)};main.Controls.Add(pageHost,0,1);foreach(string label in labels){var page=new Panel{Dock=DockStyle.Fill,AutoScroll=false,BackColor=Canvas};pages.Add(label,page);pageHost.Controls.Add(page);}
   BuildInstall();BuildUpdates();BuildBackups();details.Dock=DockStyle.Fill;details.Multiline=true;details.ReadOnly=true;details.ScrollBars=ScrollBars.Vertical;details.BackColor=Theme.Surface;details.ForeColor=Ink;details.BorderStyle=BorderStyle.None;details.Font=new Font("Consolas",9);details.AccessibleName="Setup details";details.Text="Ready. COT "+SetupInfo.Version+".";pages["Activity"].Controls.Add(details);
   status.Dock=DockStyle.Fill;status.Font=new Font(Font.FontFamily,9);status.ForeColor=Muted;status.Text="Close the game and Mod Manager before making changes.";main.Controls.Add(status,0,2);progress.Dock=DockStyle.Fill;progress.Visible=false;main.Controls.Add(progress,0,3);
   var footer=new FlowLayoutPanel{Dock=DockStyle.Fill,AutoSize=true,AutoSizeMode=AutoSizeMode.GrowAndShrink,Margin=Padding.Empty,FlowDirection=FlowDirection.RightToLeft,WrapContents=false,Padding=new Padding(0,5,0,0)};main.Controls.Add(footer,0,4);StyleButton(close,"Close",false);close.Size=new Size(88,36);footer.Controls.Add(close);StyleButton(install,"Install COT",true);install.Size=new Size(220,36);footer.Controls.Add(install);StyleButton(copy,"Copy details",false);copy.Size=new Size(112,36);footer.Controls.Add(copy);AcceptButton=install;CancelButton=close;
   browse.Click+=(sender,e)=>{using(var picker=new OpenFileDialog{Title="Choose MMCModManager.exe",Filter="MMC Mod Manager|MMCModManager.exe|Programs|*.exe",CheckFileExists=true,AutoUpgradeEnabled=true,RestoreDirectory=true}){if(Directory.Exists(target.Text))picker.InitialDirectory=target.Text;if(picker.ShowDialog(this)==DialogResult.OK)try{SetTarget(MmcLocator.FromExe(picker.FileName));}catch(IOException error){status.Text=error.Message;}}};
   target.Leave+=(s,e)=>RefreshTarget();target.TextChanged+=(s,e)=>{rootError="Choose or verify your MMC Mod Manager folder.";UpdateButtons();};operation.SelectedIndexChanged+=(s,e)=>ApplyOperation();install.Click+=(s,e)=>Begin(false);restore.Click+=(s,e)=>Begin(true);close.Click+=(s,e)=>Close();copy.Click+=(s,e)=>{try{Clipboard.SetText(details.Text);}catch(System.Runtime.InteropServices.ExternalException){status.Text="Clipboard is busy. Select and copy Activity details.";}};
   ConfigureWorkers();FormClosing+=(s,e)=>{if(busy){e.Cancel=true;status.Text="Please wait for the current operation to finish before closing setup.";}};FormClosed+=(s,e)=>{engine.Dispose();if(downloads!=null)try{Directory.Delete(downloads,true);}catch(IOException){}catch(UnauthorizedAccessException){}};
   try{updates=new UpdateClient(SetupInfo.UpdateRepository);}catch(IOException e){releaseInfo.Text=e.Message;}
   ShowPage("Install");ApplyOperation();if(fixture!=null)SetTarget(fixture.Root);else{string found=MmcLocator.Find(AppDomain.CurrentDomain.BaseDirectory);if(found!=null)SetTarget(found);else{hint.Text="Click Find exe to choose MMCModManager.exe.";UpdateButtons();}}
  }
  private static Label LabelAt(string text,int x,int y,int width,int height,float size,FontStyle style,Color color){return new Label{Text=text,Location=new Point(x,y),Size=new Size(width,height),Font=new Font("Segoe UI",size,style),ForeColor=color};}
  private static void StyleButton(Button b,string text,bool primary){b.Text=text;if(b is RoundedButton)((RoundedButton)b).Primary=primary;b.FlatStyle=FlatStyle.Flat;b.FlatAppearance.BorderColor=primary?Green:Line;b.BackColor=primary?Green:Theme.Surface;b.ForeColor=primary?Color.White:Ink;b.Cursor=Cursors.Hand;b.UseVisualStyleBackColor=false;}
  private static TableLayoutPanel Stack(Panel page){var table=new TableLayoutPanel{Dock=DockStyle.Top,AutoSize=true,AutoSizeMode=AutoSizeMode.GrowAndShrink,ColumnCount=1,RowCount=0,Padding=new Padding(0,0,1,8)};table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));page.Controls.Add(table);return table;}
  private static void Row(TableLayoutPanel table,Control control,int height){int row=table.RowCount++;table.RowStyles.Add(new RowStyle(SizeType.Absolute,height));control.Dock=DockStyle.Fill;control.Margin=new Padding(0,0,0,9);table.Controls.Add(control,0,row);}
  private static Label Heading(string text){return new Label{Text=text,Font=new Font("Segoe UI",17,FontStyle.Bold),ForeColor=Ink};}
  private static Label Body(string text){return new Label{Text=text,ForeColor=Muted,Font=new Font("Segoe UI",9)};}
  private void BuildInstall(){
   var table=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=5};table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));foreach(int height in new[]{37,36,27})table.RowStyles.Add(new RowStyle(SizeType.Absolute,height));table.RowStyles.Add(new RowStyle(SizeType.Percent,100));table.RowStyles.Add(new RowStyle(SizeType.Absolute,25));pages["Install"].Controls.Add(table);
   var title=Heading("Choose your features");title.Dock=DockStyle.Fill;table.Controls.Add(title,0,0);
   operation.DropDownStyle=ComboBoxStyle.DropDownList;operation.AccessibleName="Setup action";operation.Items.AddRange(new object[]{"Install or update COT","Remove selected features","Remove all COT"});operation.SelectedIndex=0;operation.Dock=DockStyle.Fill;operation.BackColor=Theme.Surface;operation.ForeColor=Ink;table.Controls.Add(operation,0,1);
   selectionHint.ForeColor=Muted;selectionHint.Font=new Font(Font.FontFamily,9);selectionHint.Dock=DockStyle.Fill;table.Controls.Add(selectionHint,0,2);
   settings.Dock=DockStyle.Fill;settings.BackColor=Canvas;table.Controls.Add(settings,0,3);var grid=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=2,RowCount=2};grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));grid.RowStyles.Add(new RowStyle(SizeType.Percent,50));grid.RowStyles.Add(new RowStyle(SizeType.Percent,50));settings.Controls.Add(grid);
   for(int i=0;i<4;i++){var card=new RoundedPanel{Dock=DockStyle.Fill,Margin=new Padding(i%2==0?0:5,3,i%2==0?5:0,5),Padding=new Padding(12,10,10,7)};var content=new TableLayoutPanel{Dock=DockStyle.Fill,BackColor=Theme.Surface,ColumnCount=1,RowCount=2};content.RowStyles.Add(new RowStyle(SizeType.Absolute,30));content.RowStyles.Add(new RowStyle(SizeType.Percent,100));card.Controls.Add(content);modules[i]=new ModernCheckBox{Text=SetupInfo.ModuleNames[i],Checked=true,ForeColor=Ink,Font=new Font(Font.FontFamily,11,FontStyle.Bold),AccessibleName=SetupInfo.ModuleNames[i],Dock=DockStyle.Fill};modules[i].CheckedChanged+=(sender,e)=>UpdateButtons();content.Controls.Add(modules[i],0,0);var description=moduleDescriptions[i]=Body(SetupInfo.ModuleDescriptions[i]);description.Dock=DockStyle.Fill;description.Padding=new Padding(0,3,0,0);content.Controls.Add(description,0,1);int index=i;description.Click+=(sender,e)=>{if(modules[index].Enabled)modules[index].Checked=!modules[index].Checked;};grid.Controls.Add(card,i%2,i/2);}
   packageSource.ForeColor=Muted;packageSource.Font=new Font(Font.FontFamily,8);packageSource.Dock=DockStyle.Fill;table.Controls.Add(packageSource,0,4);
  }
  private void BuildUpdates(){
   var table=Stack(pages["Updates"]);table.AutoSize=false;table.Dock=DockStyle.Fill;Row(table,Heading("Updates"),34);Row(table,Body("Check only when you want to. Choose a release and read its notes before downloading. Installation is a separate button. Keeping your current version is always an option."),44);StyleButton(checkUpdates,"Check for updates",false);checkUpdates.AccessibleName="Check for updates";Row(table,checkUpdates,38);releases.DropDownStyle=ComboBoxStyle.DropDownList;releases.AccessibleName="Available COT releases";Row(table,releases,38);Row(table,Body("Test releases are marked. Older releases remain selectable when available. Nothing is selected automatically."),28);
   releaseInfo.Multiline=true;releaseInfo.ReadOnly=true;releaseInfo.ScrollBars=ScrollBars.Vertical;releaseInfo.BackColor=Theme.Surface;releaseInfo.ForeColor=Ink;releaseInfo.BorderStyle=BorderStyle.FixedSingle;releaseInfo.AccessibleName="Release notes and compatibility";releaseInfo.Text="No update check has run. Bundled version: "+SetupInfo.Version+".\r\nProject: https://github.com/"+SetupInfo.UpdateRepository;Row(table,releaseInfo,160);table.RowStyles[5].SizeType=SizeType.Percent;table.RowStyles[5].Height=100;StyleButton(download,"Download selected release",true);download.AccessibleName="Download selected release";StyleButton(bundled,"Use bundled "+SetupInfo.Version,false);var actions=new TableLayoutPanel{ColumnCount=2,RowCount=1};actions.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));actions.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));download.Dock=DockStyle.Fill;bundled.Dock=DockStyle.Fill;actions.Controls.Add(download,0,0);actions.Controls.Add(bundled,1,0);Row(table,actions,44);
   checkUpdates.Click+=(s,e)=>StartNetwork("check");releases.SelectedIndexChanged+=(s,e)=>{if(!loadingReleases&&releases.SelectedItem!=null)StartNetwork("inspect");};download.Click+=(s,e)=>StartNetwork("download");bundled.Click+=(s,e)=>{selectedUpdate=null;UpdatePackageSource();status.Text="Bundled version selected. Your installation has not changed.";};
  }
  private void BuildBackups(){var table=Stack(pages["Backups"]);Row(table,Heading("Undo changes"),38);Row(table,Body("Installations, updates and removals create transaction backups. Choose one to undo it. Later changes to user settings and histories are retained."),70);backups.DropDownStyle=ComboBoxStyle.DropDownList;backups.AccessibleName="COT backup";Row(table,backups,43);StyleButton(restore,"Restore selected backup",false);Row(table,restore,45);Row(table,Body("Close MMC and the game before restoring. If a managed file changed since the backup, setup stops and explains the conflict instead of replacing it."),66);}
  private void ShowPage(string name){currentPage=name;install.Visible=name=="Install"||name=="Updates"&&selectedUpdate!=null;copy.Visible=name=="Activity";foreach(var pair in pages)pair.Value.Visible=pair.Key==name;foreach(var pair in navButtons){pair.Value.BackColor=pair.Key==name?Color.FromArgb(21,76,53):Color.FromArgb(10,17,21);pair.Value.ForeColor=pair.Key==name?Color.White:Color.FromArgb(213,224,218);}pages[name].BringToFront();}
  private void ConfigureWorkers(){
   worker.WorkerReportsProgress=true;worker.ProgressChanged+=(s,e)=>Append((string)e.UserState);worker.RunWorkerCompleted+=(s,e)=>{LastSuccess=e.Error==null&&((InstallResult)e.Result).Success;if(e.Error!=null)Append(e.Error.Message);if(LastSuccess){if(fixture==null)MmcLocator.Remember(loadedRoot);status.Text=restoring?"Backup restored. Restart MMC and the game when you are ready.":runningAction=="Install"?"COT is installed. Restart MMC and the game when you are ready.":"Removal complete. Settings, histories and backups were kept.";status.ForeColor=Green;}else{status.Text=FriendlyError(e.Error==null?((InstallResult)e.Result).Details:e.Error.Message);status.ForeColor=Color.FromArgb(149,44,60);}busy=false;settings.Enabled=true;navigation.Enabled=true;progress.Visible=false;if(LastSuccess)loadedRoot=null;RefreshTarget();UpdateButtons();
    var result=e.Error==null?e.Result as InstallResult:null;
    if(!LastSuccess&&!restoring&&runningAction=="Install"&&result!=null&&result.Conflicts!=null&&result.Conflicts.Count>0){
     bool approved=ReviewStandalone!=null?ReviewStandalone(result.Conflicts):fixture==null&&ConfirmStandalone(result.Conflicts);
     if(approved)Begin(false,result.Conflicts);
    }
   };
   network.WorkerReportsProgress=true;network.ProgressChanged+=(s,e)=>{if(e.ProgressPercentage>=0){progress.Style=ProgressBarStyle.Continuous;progress.Value=Math.Min(100,e.ProgressPercentage);}status.Text=(string)e.UserState;};network.RunWorkerCompleted+=(s,e)=>{
    busy=false;settings.Enabled=true;navigation.Enabled=true;progress.Visible=false;
    if(e.Error!=null){Append(e.Error.Message);status.Text="Update action stopped. Your installation has not changed.";releaseInfo.Text=e.Error.Message;inspected=null;}
    else if(networkAction=="check"){loadingReleases=true;releases.Items.Clear();foreach(var choice in (List<ReleaseChoice>)e.Result)releases.Items.Add(choice);releases.SelectedIndex=-1;loadingReleases=false;inspected=null;releaseInfo.Text=releases.Items.Count==0?"No published COT update metadata is available. Keep this version or use a manually downloaded installer.":"Choose a release above to read verified notes and compatibility information.";status.Text="Checked releases. Nothing downloaded or installed.";}
    else if(networkAction=="inspect"){inspected=(UpdateMetadata)e.Result;releaseInfo.Text="VERSION "+inspected.Version+"\r\n\r\n"+inspected.Notes+"\r\n\r\nKNOWN COMPATIBILITY\r\n"+inspected.Compatibility;status.Text="Release details loaded. Download and install only if you choose.";}
    else{selectedUpdate=(VerifiedUpdate)e.Result;UpdatePackageSource();status.Text="Download verified. Review modules, then explicitly choose Install "+selectedUpdate.Metadata.Version+".";}UpdateButtons();
   };
  }
  private void StartNetwork(string action){
   if(busy||updates==null)return;if(action=="inspect"&&releases.SelectedItem==null)return;if(action=="download"&&inspected==null)return;networkAction=action;var release=releases.SelectedItem as ReleaseChoice;var metadata=inspected;if(action!="download")inspected=null;
   busy=true;settings.Enabled=false;navigation.Enabled=false;UpdateButtons();progress.Visible=true;progress.Style=ProgressBarStyle.Marquee;status.ForeColor=Ink;status.Text=action=="check"?"Checking published releases...":action=="inspect"?"Loading release notes and compatibility...":"Downloading selected package. Nothing is being installed.";
   DoWorkEventHandler task=null;task=(s,e)=>{try{if(action=="check")e.Result=updates.Check();else if(action=="inspect")e.Result=updates.Inspect(release);else{if(downloads==null){downloads=Path.Combine(Path.GetTempPath(),"DynastyCOT-Download-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(downloads);}var clock=System.Diagnostics.Stopwatch.StartNew();long last=-1000;e.Result=updates.Download(metadata,downloads,(received,total)=>{if(clock.ElapsedMilliseconds-last>=150){last=clock.ElapsedMilliseconds;network.ReportProgress(total>0?(int)(received*100/total):0,"Downloading "+metadata.Version+"... "+(received/1048576)+" MB received. Installation stays separate.");}});}}finally{network.DoWork-=task;}};network.DoWork+=task;network.RunWorkerAsync();
  }
  private void Append(string text){details.AppendText(Environment.NewLine+text);details.SelectionStart=details.TextLength;details.ScrollToCaret();}
  internal void SetTarget(string root){target.Text=root;RefreshTarget();}
  private void RefreshTarget(){if(busy)return;try{string root=InstallerEngine.ValidateRoot(target.Text);if(fixture!=null&&!String.Equals(root,fixture.Root,StringComparison.OrdinalIgnoreCase))throw new IOException("This test window only accepts its fixture folder.");if(target.Text!=root)target.Text=root;var selected=InstallerEngine.LoadModules(root);if(!String.Equals(loadedRoot,root,StringComparison.OrdinalIgnoreCase)){for(int i=0;i<4;i++)modules[i].Checked=operation.SelectedIndex==0?selected[InstallerEngine.ModuleIds[i]]:operation.SelectedIndex==2;loadedRoot=root;}backups.Items.Clear();foreach(var backup in InstallerEngine.Backups(root))backups.Items.Add(backup);if(backups.Items.Count>0)backups.SelectedIndex=0;rootError=null;hint.Text="Found Mod Manager. Your saves and settings are kept.";var version=InstallerEngine.InstalledVersion(root);installed.Text=version.StartsWith("Installed (older",StringComparison.Ordinal)?"Older COT found":version;}catch(Exception e){rootError=e.Message;hint.Text=e.Message;installed.Text="Not verified";backups.Items.Clear();}UpdateButtons();}
  internal Dictionary<string,bool> Selection(){var selected=new Dictionary<string,bool>();for(int i=0;i<4;i++)selected.Add(InstallerEngine.ModuleIds[i],modules[i].Checked);return selected;}
  internal void SetModule(int i,bool enabled){modules[i].Checked=enabled;}internal void SetOperation(int i){operation.SelectedIndex=i;}
  private string ActionName{get{return operation.SelectedIndex==1?"UninstallSelected":operation.SelectedIndex==2?"UninstallAll":"Install";}}
  private void ApplyOperation(){loadedRoot=null;selectionHint.Text=operation.SelectedIndex==0?"Turn on the features you want to use.":operation.SelectedIndex==1?"Check modules to remove. Other modules and user data stay.":"Remove the shared COT plugin and all four modules; keep user data and backups.";for(int i=0;i<4;i++)if(modules[i]!=null)modules[i].Enabled=operation.SelectedIndex!=2;RefreshTarget();UpdatePackageSource();}
  private void UpdatePackageSource(){install.Visible=currentPage=="Install"||currentPage=="Updates"&&selectedUpdate!=null;packageSource.Text="PACKAGE  |  "+(selectedUpdate==null?"Bundled "+SetupInfo.Version:selectedUpdate.Metadata.Version+" - downloaded and SHA-256 verified");install.Text=operation.SelectedIndex==0?(selectedUpdate==null?"Install "+SetupInfo.Version:"Install "+selectedUpdate.Metadata.Version):operation.SelectedIndex==1?"Uninstall selected":"Uninstall all COT";}
  internal void ClickInstall(){ShowPage("Install");install.PerformClick();}internal void ClickRestore(){ShowPage("Backups");restore.PerformClick();}
  private void Begin(bool rollback){Begin(rollback,null);}
  private void Begin(bool rollback,IList<StandaloneConflict> reviewed){
   if(busy)return;RefreshTarget();if(rootError!=null){status.Text=rootError;return;}var backup=backups.SelectedItem as Backup;if(rollback&&backup==null)return;string root=InstallerEngine.ValidateRoot(target.Text),backupPath=rollback?backup.Path:null;var selected=Selection();var update=selectedUpdate;
   runningAction=ActionName;busy=true;restoring=rollback;LastSuccess=false;settings.Enabled=false;navigation.Enabled=false;UpdateButtons();progress.Style=ProgressBarStyle.Marquee;progress.Visible=true;status.ForeColor=Ink;status.Text=rollback?"Restoring selected backup...":runningAction=="Install"?"Installing selected package and creating a backup...":"Removing selected COT files and creating a backup...";details.Clear();DoWorkEventHandler task=null;task=(s,e)=>{try{e.Result=engine.Run(root,selected,backupPath,fixture,line=>worker.ReportProgress(0,line),runningAction,update,reviewed);}finally{worker.DoWork-=task;}};worker.DoWork+=task;worker.RunWorkerAsync();
  }
  private void UpdateButtons(){bool any=false;foreach(var module in modules)if(module!=null&&module.Checked)any=true;install.Enabled=!busy&&rootError==null&&(operation.SelectedIndex!=1||any);restore.Enabled=!busy&&rootError==null&&backups.Items.Count>0;backups.Enabled=!busy&&backups.Items.Count>0;close.Enabled=!busy;browse.Enabled=!busy;target.Enabled=!busy;checkUpdates.Enabled=!busy&&updates!=null;releases.Enabled=!busy;download.Enabled=!busy&&inspected!=null;bundled.Enabled=!busy;}
  private bool ConfirmStandalone(List<StandaloneConflict> conflicts){
   var names=new List<string>();for(int i=0;i<Math.Min(8,conflicts.Count);i++)names.Add(conflicts[i].Path);
   string more=conflicts.Count>8?"\r\n...and "+(conflicts.Count-8)+" more (full list in Details).":"";
   return MessageBox.Show(this,"These older standalone plugin files are not recognized:\r\n\r\n"+String.Join("\r\n",names.ToArray())+more+
    "\r\n\r\nBack up these exact files and replace them with COT? Settings and histories stay. Undo changes can restore the originals. Choose No to leave everything unchanged.","Replace older standalone copies?",MessageBoxButtons.YesNo,MessageBoxIcon.Question,MessageBoxDefaultButton.Button2)==DialogResult.Yes;
  }
  internal static string FriendlyError(string text){
   if(text.Contains("COT_STANDALONE_CONFLICTS:")||text.Contains("Unrecognized active legacy payload kept"))return "Older standalone copies need review. Files are unchanged; open Details.";
   if(text.Contains("COT_RUNNING_PROCESSES:")||text.Trim()=="Close MMC and the game before installing or rolling back")return "Close MMC and the game, then try again.";
   if(text.Contains("COT_WRITE_ACCESS:"))return "Cannot write to the selected Mod Manager folder. Open Details for the path and next steps.";
   if(text.Contains("restored:"))return "Installation stopped; previous files were restored. Open Details.";
   if(text.Contains("Conflicting Enhancer history"))return "The two history copies differ. Both were kept; open Details.";
   if(text.Contains("changed after installation"))return "A managed file changed since this backup. Nothing was restored.";
   return "Setup could not finish. Open Details for the specific next step.";
  }

  // Absolute sidebar/header rows scale with DPI; the page host takes the remaining
  // space. Never let the scaled minimum push the persistent footer
  // below the monitor's usable desktop (taskbar excluded).
  private void FitWorkingArea(Rectangle area){
   if(fitting||WindowState!=FormWindowState.Normal)return;fitting=true;
   try{lastWorkingArea=area;const int margin=16;var available=new Size(Math.Max(200,area.Width-margin*2),Math.Max(200,area.Height-margin*2));
    MinimumSize=new Size(Math.Min(MinimumSize.Width,available.Width),Math.Min(MinimumSize.Height,available.Height));
    Size=new Size(Math.Min(Width,available.Width),Math.Min(Height,available.Height));
    Location=new Point(Math.Max(area.Left+margin,Math.Min(Left,area.Right-margin-Width)),Math.Max(area.Top+margin,Math.Min(Top,area.Bottom-margin-Height)));
   }finally{fitting=false;}
  }
  private Rectangle CurrentWorkingArea{get{return layoutArea??Screen.FromControl(this).WorkingArea;}}
  protected override void OnShown(EventArgs e){FitWorkingArea(CurrentWorkingArea);base.OnShown(e);}
  protected override void OnLocationChanged(EventArgs e){base.OnLocationChanged(e);if(IsHandleCreated&&!fitting&&CurrentWorkingArea!=lastWorkingArea)FitWorkingArea(CurrentWorkingArea);}
  protected override void WndProc(ref Message message){base.WndProc(ref message);if((message.Msg==0x02E0||message.Msg==0x007E)&&IsHandleCreated)FitWorkingArea(CurrentWorkingArea);}
  private void CheckFooterLayout(){
   foreach(Control action in new Control[]{install,close,copy,status})if(action.Visible){
    Rectangle visible=action.RectangleToScreen(action.ClientRectangle);
    for(Control parent=action.Parent;parent!=null;parent=parent.Parent)if(!parent.RectangleToScreen(parent.ClientRectangle).Contains(visible))throw new IOException("Footer is clipped by its parent: "+action.Text);
    if(action is Button){Size text=TextRenderer.MeasureText(action.Text,action.Font,Size.Empty,TextFormatFlags.SingleLine);if(text.Width>action.ClientSize.Width||text.Height>action.ClientSize.Height)throw new IOException("Footer button text does not fit: "+action.Text);}
   }
  }
  internal Dictionary<string,object> CaptureLayout(string folder,float scale,Rectangle? simulatedArea=null){
   if(fixture==null)throw new IOException("Layout capture requires a marked fixture.");layoutArea=simulatedArea;if(scale!=1){var fonts=new Dictionary<Control,Font>();Action<Control> collect=null;collect=c=>{fonts[c]=new Font(c.Font.FontFamily,c.Font.Size*scale,c.Font.Style);foreach(Control child in c.Controls)collect(child);};collect(this);Scale(new SizeF(scale,scale));foreach(var item in fonts)item.Key.Font=item.Value;}CreateControl();Show();
   Rectangle area=simulatedArea??Screen.FromControl(this).WorkingArea;FitWorkingArea(area);PerformLayout();Application.DoEvents();
   bool brandImage=false;Action<Control> inspect=null;inspect=control=>{if(control is PictureBox)brandImage=true;foreach(Control child in control.Controls)inspect(child);};inspect(this);
   if(brandImage||Icon==null)throw new IOException("The window must retain its app icon without a large in-app image.");
   Rectangle allowed=Rectangle.Inflate(area,-16,-16);if(!allowed.Contains(Bounds))throw new IOException("Scaled setup exceeds its working area.");
   string suffix=(int)(scale*100)+(simulatedArea.HasValue?"-work"+area.Height:"");
   foreach(string page in new[]{"Install","Updates","Backups","Activity"}){
    ShowPage(page);PerformLayout();Application.DoEvents();
    CheckFooterLayout();
    if(page=="Updates")foreach(Control action in new Control[]{checkUpdates,releases,download,bundled}){Rectangle rect=pages[page].RectangleToClient(action.RectangleToScreen(action.ClientRectangle));if(!pages[page].ClientRectangle.Contains(rect))throw new IOException("Update action is outside the scaled page.");}
    using(var image=new Bitmap(Width,Height)){DrawToBitmap(image,new Rectangle(Point.Empty,Size));image.Save(Path.Combine(folder,"setup-"+page.ToLowerInvariant()+"-"+suffix+".png"));}
   }ShowPage("Install");PerformLayout();Application.DoEvents();
   foreach(var module in modules){Rectangle card=pages["Install"].RectangleToClient(module.Parent.RectangleToScreen(module.Parent.ClientRectangle));if(!pages["Install"].ClientRectangle.Contains(card))throw new IOException("Every module must be visible without scrolling.");}
   foreach(var label in moduleDescriptions){Size available=new Size(label.ClientSize.Width-label.Padding.Horizontal,label.ClientSize.Height-label.Padding.Vertical);Size text=TextRenderer.MeasureText(label.Text,label.Font,new Size(available.Width,Int32.MaxValue),TextFormatFlags.WordBreak|TextFormatFlags.TextBoxControl);if(text.Height>available.Height)throw new IOException("Module description is clipped: "+label.Text);}
   using(var image=new Bitmap(Width,Height)){DrawToBitmap(image,new Rectangle(Point.Empty,Size));image.Save(Path.Combine(folder,"setup-install-scrolled-"+suffix+".png"));}
   return new Dictionary<string,object>{{"suffix",suffix},{"scale",(int)(scale*100)},{"workingWidth",area.Width},{"workingHeight",area.Height},{"width",Width},{"height",Height},{"footerVisible",true},{"updateActionsVisible",true},{"allModulesReachable",true},{"moduleDescriptionsFit",true},{"fontScale",(int)(scale*100)},{"footerParentContainment",true},{"footerTextFits",true},{"inAppBrandImage",brandImage},{"windowIconRetained",Icon!=null}};
  }
 }
}

