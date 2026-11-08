param([Parameter(Mandatory=$true)][string]$Archive,[switch]$EmbeddedLoader)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$path=(Resolve-Path -LiteralPath $Archive).Path
$zip=[IO.Compression.ZipFile]::OpenRead($path)
try {
 $dllEntry=if($EmbeddedLoader){'Plugins/DynamicGamedayAtmosphere.dll'}else{'DynamicGamedayAtmosphere.dll'}
 $wanted=@('FEATURES.md',$dllEntry,'README.md')
 if(-not $EmbeddedLoader){
  $loaders=@($zip.Entries | Where-Object {$_.FullName.Replace('\','/') -match '^DynamicGamedayAtmosphere/Runtime-(\d+\.\d+\.\d+)/LoadDynamicGamedayAtmosphere\.exe$'})
  if($loaders.Count -gt 1){throw 'Unexpected extra runtime loaders.'}
  $version=''
  if($loaders.Count -eq 1){
   $wanted+=@($loaders[0].FullName.Replace('\','/'))
   $version=[regex]::Match($loaders[0].FullName,'Runtime-(\d+\.\d+\.\d+)').Groups[1].Value
  }
  $wanted=@($wanted | Sort-Object)
  # Reflection-only inspection runs under .NET Framework even when the calling
  # PowerShell uses .NET Core. No packaged assembly code is executed.
  $verifierFolder=Join-Path (Split-Path -Parent $PSScriptRoot) 'Build\PackageVerifier'
  New-Item -ItemType Directory -Path $verifierFolder -Force | Out-Null
  $verifier=Join-Path $verifierFolder 'VerifyPackageResources.exe'
  & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll ('/out:'+$verifier) (Join-Path $PSScriptRoot 'VerifyPackageResources.cs')
  if($LASTEXITCODE -ne 0){throw 'Package resource verifier compilation failed.'}
  & $verifier $path $version
  if($LASTEXITCODE -ne 0){throw 'Package runtime resource/hash verification failed.'}
 }
 $actual=@($zip.Entries | ForEach-Object {$_.FullName.Replace('\','/')} | Sort-Object)
 if(($actual -join '|') -ne ($wanted -join '|')){throw ('Unexpected ZIP contents: '+($actual -join ', '))}
 foreach($entry in $zip.Entries){
  if($entry.Length -le 0){throw ('Empty entry: '+$entry.FullName)}
  if($entry.FullName.EndsWith('.md')){
   $reader=New-Object IO.StreamReader($entry.Open())
   try {$text=$reader.ReadToEnd()}finally{$reader.Dispose()}
   if($text -match '(?i)DynamicAttendance\.dll'){throw 'A retired DLL name appears in public documentation.'}
  }
 }
}finally{$zip.Dispose()}
Write-Output ('PASS: package layout, documentation and runtime resources/hash. SHA256 '+(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant())
