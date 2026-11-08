param([ValidateSet('Baseline','Core','Enhancer','All')][string]$Suite='Baseline')
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
function Verify-Baseline {
    $manifest=Get-Content -LiteralPath (Join-Path $root 'Baselines/source-files.json') -Raw | ConvertFrom-Json
    $changes=Get-Content -LiteralPath (Join-Path $root 'Baselines/changes.json') -Raw | ConvertFrom-Json
    $declared=@{}
    foreach($change in $changes.changes){
        if(!$change.path -or !$change.sha256 -or !$change.reason){throw 'Each deliberate source change needs path, sha256 and reason.'}
        if($declared.ContainsKey($change.path)){throw ('Duplicate change: '+$change.path)}
        $declared[$change.path]=$change
    }
    foreach($entry in $manifest.files){
        $path=[IO.Path]::GetFullPath((Join-Path $root $entry.path))
        if(!$path.StartsWith($root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Baseline path escapes repository.'}
        if(!(Test-Path -LiteralPath $path -PathType Leaf)){throw ('Missing source: '+$entry.path)}
        $actual=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
        $expected=if($declared.ContainsKey($entry.path)){$declared[$entry.path].sha256}else{$entry.sha256}
        if($actual -ne $expected){throw ('Undeclared or changed source: '+$entry.path)}
    }
    foreach($key in $declared.Keys){if(!($manifest.files.path -contains $key)){throw ('Change is not an imported source: '+$key)}}
    Write-Output ('PASS: '+$manifest.files.Count+' imported source files verified.')
}
function Run-Checked([string]$Name,[scriptblock]$Action){
    Write-Output ('Running '+$Name)
    & $Action
    if($LASTEXITCODE -ne 0){throw ($Name+' failed with exit code '+$LASTEXITCODE)}
}
Verify-Baseline
if($Suite -in @('Core','All')){
    Push-Location (Join-Path $root 'Modules/ExtraDynastySettings')
    try { Run-Checked 'Extra Dynasty Settings core' { & pwsh -NoProfile -File './Test.ps1' } } finally { Pop-Location }
    Push-Location (Join-Path $root 'Modules/DrivePractice')
    try { Run-Checked 'Drive Practice core' { & pwsh -NoProfile -File './Test.ps1' } } finally { Pop-Location }
}
if($Suite -in @('Enhancer','All')){
    Push-Location (Join-Path $root 'Modules/DynastyEnhancer')
    try {
        $tests=@(Get-ChildItem -LiteralPath 'Tests' -Filter '*.test.mjs' -File | Sort-Object Name | ForEach-Object FullName)
        if(!$tests.Count){throw 'No Enhancer Node tests found.'}
        Run-Checked 'Dynasty Enhancer Node tests' { & node --test @tests }
    } finally {Pop-Location}
}
Write-Output ('PASS: requested '+$Suite+' checks. No live game validation was performed.')
