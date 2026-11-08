$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$programFilesX86=[Environment]::GetFolderPath('ProgramFilesX86')
$vs=& (Join-Path $programFilesX86 'Microsoft Visual Studio/Installer/vswhere.exe') -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$tool=Get-ChildItem -LiteralPath (Join-Path $vs 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$kits=Join-Path $programFilesX86 'Windows Kits/10'
$sdk=Get-ChildItem -LiteralPath (Join-Path $kits 'Include') -Directory | Where-Object {Test-Path -LiteralPath (Join-Path $_.FullName 'ucrt/stdlib.h')} | Sort-Object Name -Descending | Select-Object -First 1
$includes=@(('/I'+(Join-Path $tool.FullName 'include')),('/I'+(Join-Path $sdk.FullName 'ucrt')),('/I'+(Join-Path $sdk.FullName 'shared')),('/I'+(Join-Path $sdk.FullName 'um')))
$libs=@(('/LIBPATH:'+(Join-Path $tool.FullName 'lib/x64')),('/LIBPATH:'+(Join-Path $kits ('Lib/'+$sdk.Name+'/ucrt/x64'))),('/LIBPATH:'+(Join-Path $kits ('Lib/'+$sdk.Name+'/um/x64'))))
$out=Join-Path $root 'Build/Compatibility'
Push-Location $out
try{
 & (Join-Path $tool.FullName 'bin/Hostx64/x64/cl.exe') /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @includes (Join-Path $root 'Tests/PracticeContractChecks.cpp') /Fe:PracticeContractChecks.exe /link @libs
 if($LASTEXITCODE -ne 0){throw 'Practice contract checks did not compile'}
 & './PracticeContractChecks.exe'
 if($LASTEXITCODE -ne 0){throw 'Practice contract checks failed'}
}finally{Pop-Location}
