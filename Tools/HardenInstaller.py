from pathlib import Path
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT')
p=root/'Packaging/Install.ps1';s=p.read_text()
needle="function Restore-Transaction([string]$Directory,[switch]$Automatic){"
atomic="""function Write-AtomicJournal([string]$Path,$Value){
 $temporary=$Path+'.tmp'
 $text=$Value|ConvertTo-Json -Depth 7
 try{
  $stream=New-Object IO.FileStream($temporary,[IO.FileMode]::Create,[IO.FileAccess]::Write,[IO.FileShare]::None)
  try{$bytes=$utf8.GetBytes($text);$stream.Write($bytes,0,$bytes.Length);$stream.Flush($true)}finally{$stream.Dispose()}
  if(Test-Path -LiteralPath $Path){[IO.File]::Replace($temporary,$Path,$null)}else{[IO.File]::Move($temporary,$Path)}
 }finally{if(Test-Path -LiteralPath $temporary){Remove-Item -LiteralPath $temporary -Force}}
}
"""
s=s.replace(needle,atomic+needle)
s=s.replace("[IO.File]::WriteAllText($journalPath,($journal|ConvertTo-Json -Depth 7),$utf8)","Write-AtomicJournal $journalPath $journal")
s=s.replace("[IO.File]::WriteAllText((Join-Path $directory 'transaction.json'),($journal|ConvertTo-Json -Depth 7),$utf8)","Write-AtomicJournal (Join-Path $directory 'transaction.json') $journal")
s=s.replace(" Fault 'AfterLegacyDisable'",""" # Retire only recognized old native payload names, never history or another mod.
 foreach($file in Get-ChildItem -LiteralPath (Join-Path $MmcRoot 'Plugins') -Recurse -File -Filter '*.dll'){
  $relative=$file.FullName.Substring($MmcRoot.Length+1).Replace('\\','/')
  if($relative -match '^Plugins/(DynastyEnhancer/Runtime-[^/]+/|DynamicGamedayAtmosphere/Runtime-[^/]+/)?(DynastyEnhancerNative[^/]*|DynastyEnhancerRecruitFog[^/]*|DynamicGamedayAtmosphereNative[^/]*|DynamicAttendanceNative[^/]*)[.]dll$'){
   Record-Target $relative $false | Out-Null
   Remove-Item -LiteralPath $file.FullName -Force
  }
 }
 Fault 'AfterLegacyDisable'""")
# Serialize installer/rollback operations per selected MMC root without a lock file.
needle="if($Rollback){"
lock="""$hasher=[Security.Cryptography.SHA256]::Create()
try{$key=[BitConverter]::ToString($hasher.ComputeHash($utf8.GetBytes($MmcRoot.ToUpperInvariant()))).Replace('-','')}finally{$hasher.Dispose()}
$mutex=New-Object Threading.Mutex($false,('Local\\DynastyCOT.Install.'+$key))
try{$owned=$mutex.WaitOne(0)}catch [Threading.AbandonedMutexException]{$owned=$true}
if(!$owned){$mutex.Dispose();throw 'Another COT installer/rollback owns this MMC folder'}
try{
"""
s=s.replace(needle,lock+needle,1)
s+="\n}finally{$mutex.ReleaseMutex();$mutex.Dispose()}\n"
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'Tests/test_installer.py';s=p.read_text()
needle="  write(root,'Plugins/DynastyCOT.dll',b'previous COT adapter')"
s=s.replace(needle,"  write(root,'Plugins/DynastyEnhancer/Runtime-v103/DynastyEnhancerNative-v103.dll',b'old native payload')\n  write(root,'Plugins/DynamicGamedayAtmosphere/Runtime-1.54.0/DynamicGamedayAtmosphereNative-1.54.0.dll',b'old atmosphere payload')\n"+needle)
needle="check((upgrade/'COT-Runtime/DynastyEnhancer/RuntimeHistory/101/history.json').read_bytes()"
s=s.replace(needle,"check([p.name for p in (upgrade/'Plugins').rglob('*.dll')]==['DynastyCOT.dll'],'legacy native DLLs remain in recursive scan')\n"+needle)
p.write_text(s,encoding='utf-8',newline='\n')
print('Added atomic journal, installation mutex and narrow legacy-native retirement')
