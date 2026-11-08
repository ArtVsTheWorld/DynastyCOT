function Get-DgaWindowsSdk {
    param([Parameter(Mandatory=$true)][string]$KitsRoot)
    $include=Join-Path $KitsRoot 'Include'
    if(-not(Test-Path -LiteralPath $include)){throw 'A Windows 10/11 SDK is required.'}
    $sdk=Get-ChildItem -LiteralPath $include -Directory | Where-Object {
        $candidate=$_
        $candidate.Name -match '^\d+\.\d+\.\d+\.\d+$' -and
        (Test-Path -LiteralPath (Join-Path $candidate.FullName 'um\Windows.h')) -and
        (Test-Path -LiteralPath (Join-Path $candidate.FullName 'ucrt\stdio.h')) -and
        (Test-Path -LiteralPath (Join-Path $candidate.FullName 'shared\sdkddkver.h')) -and
        (Test-Path -LiteralPath (Join-Path $KitsRoot ('Lib\'+$candidate.Name+'\um\x64\kernel32.lib'))) -and
        (Test-Path -LiteralPath (Join-Path $KitsRoot ('Lib\'+$candidate.Name+'\ucrt\x64\ucrt.lib')))
    } | Sort-Object {[version]$_.Name} -Descending | Select-Object -First 1
    if(-not $sdk){throw 'No complete x64 Windows SDK was found. Install a Windows SDK with C++ headers and x64 libraries.'}
    return $sdk
}
