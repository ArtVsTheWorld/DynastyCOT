param([string]$Version='', [switch]$Automatic, [switch]$CotBranding)
$ErrorActionPreference='Stop'
$product=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'Product.json') -Raw | ConvertFrom-Json
if(-not $Version){$Version='Release'+$product.version.Replace('.','')}
if($product.version -notmatch '^\d+\.\d+\.\d+$'){throw 'Invalid product version'}
if($Version -notmatch '^[A-Za-z0-9-]+$'){throw 'Invalid build name'}
$crowdRoot=Split-Path -Parent $PSScriptRoot
$crowdOut=Join-Path $crowdRoot ('Build\'+$Version)
New-Item -ItemType Directory -Path $crowdOut -Force | Out-Null
$crowdVsWhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if(-not(Test-Path -LiteralPath $crowdVsWhere)){throw 'Install Visual Studio Build Tools with Desktop development with C++ and a Windows SDK.'}
$crowdVs=& $crowdVsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(-not $crowdVs){throw 'Visual Studio C++ tools were not found. Add Desktop development with C++ in Visual Studio Installer.'}
$crowdTool=Get-ChildItem -LiteralPath (Join-Path $crowdVs 'VC\Tools\MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
$crowdKits=Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
. (Join-Path $PSScriptRoot 'BuildTools.ps1')
$crowdSdk=Get-DgaWindowsSdk -KitsRoot $crowdKits
Write-Output ('Using MSVC '+$crowdTool.Name+' and Windows SDK '+$crowdSdk.Name)
$crowdCl=Join-Path $crowdTool.FullName 'bin\Hostx64\x64\cl.exe'
$crowdMl=Join-Path $crowdTool.FullName 'bin\Hostx64\x64\ml64.exe'
$crowdInclude=@(('/I'+(Join-Path $crowdTool.FullName 'include')),('/I'+(Join-Path $crowdSdk.FullName 'ucrt')),('/I'+(Join-Path $crowdSdk.FullName 'shared')),('/I'+(Join-Path $crowdSdk.FullName 'um')))
$crowdLib=@(('/LIBPATH:'+(Join-Path $crowdTool.FullName 'lib\x64')),('/LIBPATH:'+(Join-Path $crowdKits ('Lib\'+$crowdSdk.Name+'\ucrt\x64'))),('/LIBPATH:'+(Join-Path $crowdKits ('Lib\'+$crowdSdk.Name+'\um\x64'))))
$productHeader='#pragma once'+[Environment]::NewLine+'#define GAMEDAY_VERSION "'+$product.version+'"'+[Environment]::NewLine+'#define GAMEDAY_SCHEMA '+$product.schema+[Environment]::NewLine
Set-Content -LiteralPath (Join-Path $crowdOut 'ProductVersion.h') -Value $productHeader -Encoding ascii
$nativePrefix=if($CotBranding){'GamedayAtmosphereNative-'}else{'DynamicGamedayAtmosphereNative-'}
$buildInfo='namespace DynamicGamedayAtmosphere { internal static class BuildInfo { public const string Version="'+$product.version+'"; public const string AssemblyVersion="'+$product.version+'.0"; public const string NativeFile="'+$nativePrefix+$product.version+'.dll"; } }'
Set-Content -LiteralPath (Join-Path $crowdOut 'BuildInfo.cs') -Value $buildInfo -Encoding ascii
$crowdInclude+=('/I'+$crowdOut)
$crowdBindings=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'GameBindings.h') -Raw
$crowdSites=@('StartingAttendanceFormula','StartingAttendanceLaunch','StartingAttendanceBoundary','CrowdVisibility','PeriodChange','CrowdPartResolve','AttendanceUpdate','PresentationUpdate','CurrentDownGetter','CurrentOffenseGetter','SackPredicate','ForcedFumblePredicate','AudioUpdate','IntensityUpdate','CameraViewCopy','ReplayUpdate','PauseMenuText','ReplayEventNodeUpdate','ReplayNodeUpdate','PresentationClient','PresentationEvent')
$crowdReportSites=foreach($site in $crowdSites){
    $match=[regex]::Match($crowdBindings,'\b'+[regex]::Escape($site)+'\s*=\s*0x([0-9a-fA-F]+)')
    if(-not $match.Success){throw ('Missing compatibility-report binding: '+$site)}
    $site+' '+$match.Groups[1].Value
}
$crowdReportFile=Join-Path $crowdOut 'CompatibilitySites.txt'
Set-Content -LiteralPath $crowdReportFile -Value $crowdReportSites -Encoding ascii
Push-Location $crowdOut
try {
    $crowdObjects=@()
    if(-not $Automatic){
        & $crowdMl /nologo /c (Join-Path $PSScriptRoot 'TickGate.asm')
        if($LASTEXITCODE -ne 0){throw 'Tick gate build failed'}
        $crowdObjects=@('TickGate.obj')
    }
    $crowdSource=Join-Path $PSScriptRoot 'Probe.cpp'
    $crowdDll='DynamicAttendanceProbe-'+$Version+'.dll'
    if($Automatic){$crowdSource=Join-Path $PSScriptRoot 'NativeRuntime.cpp';$crowdDll=$nativePrefix+$product.version+'.dll'}
    & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 /DUNICODE /D_UNICODE /Zi @crowdInclude /LD $crowdSource @crowdObjects ('/Fe:'+$crowdDll) /link @crowdLib /DEBUG /INCREMENTAL:NO
    if($LASTEXITCODE -ne 0){throw 'Probe build failed'}
    if($Automatic){
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tests\AudienceCorrectionTests.cpp') /Fe:AudienceCorrectionTests.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Audience correction checks build failed'}
        & '.\AudienceCorrectionTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Audience correction checks failed'}
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tests\HostileOpeningResultTests.cpp') /Fe:HostileOpeningResultTests.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Hostile opening response checks build failed'}
        & '.\HostileOpeningResultTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Hostile opening response checks failed'}
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tests\RulesTests.cpp') /Fe:RulesTests.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Rule checks build failed'}
        & '.\RulesTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Rule checks failed'}
        foreach($crowdTest in @('NativeAppearanceTests','VisitorPlanTests','AwayCrowdTests','VisitorHostStrengthTests','VisitorDemandTests','VisitorInfluenceTests','AwayPartyTests','AwayCrowdCalibrationTests','AwayCrowdIntegrationTests','BandProtectionTests','ProtectedFanBudgetTests','ProtectedFanBudgetRuntimeTests','FumbleOwnershipTests','RuntimePathsTests','AudienceModelTests','NativeAudienceTests','NativeReadTests','CameraProjectionTests','GameplayCameraTests','Tuning2020Tests','AttendanceProfileTests','BackgroundPresenceTests','SkillOverlapTests','DenseCrowdTests','SmoothGainTests','ReactionPaletteTests','AwaySoundReactionsTests','ShakeRebalanceTests','Attendance202Tests','Cleanup1990Tests','NightEngagementTests','LowerBowlVariationTests','ShowboatingTests','PopulationReactionsTests','DefensiveMotionTests','CloseGameRetentionTests','VenueMotionTests','FinalReactionCarryTests','SackCarryoverTests','ResumeHistoryTests','StartingAttendanceTransitionTests','StartingAttendanceLaunchHookTests','CrowdCohesionTests','RankedRivalryOpeningTests','StartingAttendanceTests','NativeStartingAttendanceTests','ReplaySelectionTraceTests','RainyHomeWinTests','ScoringMotionTests','TurnoverDownsTests','MajorReplayTests','SoundOnlyReactionsTests','AwayTakeawayTests','ClockManagementTests','CannonImpactTests','MotionSoundCoverageTests','ReturnRampTests','ReactionInteractionTests','CatchKnockoutTests','DefensiveAccentTests','SkillAccentTests','SettingsMenuTests','SettingsAptRoutingTests','ScoringHandoffTests','UpsetSoundTests','TrafficRushTests','LoaderDiagnosticsTests','NativeSkillSignalsTests','ContextReactionsTests','FourthAndAccentTests','CohesiveReactionsTests','CallbackGateTests','BlockedFieldGoalTests','NativeReactionSequenceTests','FumbleRecoveryFallbackTests','FumbleContactTests','HostileOpeningTests','DefensiveLossImpactTests','NativeReactionTraceTests','ReactionShapeTests','OpeningDriveTests','PauseMenuTests','AttendanceMixerTests','StadiumSilenceTests','NativeAtmosphereContextTests','AttendanceAudioTests','PresentationCameraTests','HandoffCameraTests','DirectorShakeTests','DirectorShakeRuntimeTests','LiveReactionsTests','RunBuildupTests','RunSoundscapeTests','CatchImpactTests','VenueVolumeTests','LateTrafficTests','QuarterBreakGroupTests','SituationalReactionsTests','LateHalfAtmosphereTests','LiveFumbleReturnTests','FinalFeedbackTests','OpeningKickoffTests','OpeningSeriesTests','KickoffSoundscapeTests','CameraCutContinuityTests','PostWhistleHoldTests','SealingTakeawayTests','SafetyReactionTests','PlayReactionTimingTests','DefensiveStopEligibilityTests','DiagnosticPreviewTests','ImmediateCatchTests','LineupAtmosphereTests','BigGameEnergyTests','LineupSustainTests','ReactionVarietyTests','ReactiveDepthTests','PostPlayAtmosphereTests','StadiumOccupancyTests','RenderedOccupancyTests','DrivePackedHouseTests','PackedHouseTests','SizeAtmosphereTests','CameraAdmissionTests','PassFlightTests','CoreAtmosphereTests','CrowdAudioTests','CrowdAudioRuntimeTests','ChantMixerTests','ChantRuntimeTests','AtmosphereTests','EnergyTests','AtmosphereBalanceTests','OpeningAndLimitsTests','MomentumOvertimeTests','DepartureModelTests','AttendanceBalanceTests','ArrivalTests','VisibilityDispatchTests','LoggingTests','SettingsPollTests','SustainedCrowdTests','SwellRuntimeTests','ReactionRuntimeTests','CrowdIntensityTests','PresentationAtmosphereTests','SparseHfaCameraTests','CrowdChangeWindowTests','AttendanceTimelineTests','ResumeBridgeTests','ReplayUpdateHookTests','ReplayPresentationTests','SeatPatternTests','SeatBaselineTests','SeatRuntimeTests','GamedayFlowTests')){
            & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot ('Tests\'+$crowdTest+'.cpp')) ('/Fe:'+$crowdTest+'.exe') /link @crowdLib
            if($LASTEXITCODE -ne 0){throw ($crowdTest+' build failed')}
            & ('.\'+$crowdTest+'.exe')
            if($LASTEXITCODE -ne 0){throw ($crowdTest+' failed')}
        }
        foreach($auditTest in @('NativeMenuReadTests','ConfigurationAuditTests')){
            & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot ('Tests\'+$auditTest+'.cpp')) ('/Fe:'+$auditTest+'.exe') /link @crowdLib
            if($LASTEXITCODE -ne 0){throw ($auditTest+' build failed')}
            & ('.\'+$auditTest+'.exe')
            if($LASTEXITCODE -ne 0){throw ($auditTest+' failed')}
        }
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tools\AudioMixExamples.cpp') /Fe:AudioMixExamples.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Audio range examples build failed'}
        & '.\AudioMixExamples.exe' (Join-Path $crowdOut 'AudioMixExamples.csv')
        if($LASTEXITCODE -ne 0){throw 'Audio range examples failed'}
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tools\CompareStartingAttendance.cpp') /Fe:CompareStartingAttendance.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Starting attendance examples build failed'}
        & '.\CompareStartingAttendance.exe' | Set-Content -LiteralPath 'StartingAttendanceExamples.csv' -Encoding utf8
        if($LASTEXITCODE -ne 0){throw 'Starting attendance examples failed'}
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tools\SimulateAttendance.cpp') /Fe:SimulateAttendance.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Attendance simulation build failed'}
        & '.\SimulateAttendance.exe' (Join-Path $crowdOut 'SimulationChecks')
        if($LASTEXITCODE -ne 0){throw 'Attendance simulation checks failed'}
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 @crowdInclude (Join-Path $crowdRoot 'Tools\CompareScheduledAttendance.cpp') /Fe:CompareScheduledAttendance.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Scheduled attendance analysis build failed'}
        & $crowdCl /nologo /O2 /MT /W4 /WX /EHsc /std:c++17 /DUNICODE /D_UNICODE @crowdInclude (Join-Path $PSScriptRoot 'LoadRuntime.cpp') /Fe:LoadDynamicGamedayAtmosphere.exe /link @crowdLib
        if($LASTEXITCODE -ne 0){throw 'Loader build failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /out:LauncherDiagnosticsTests.exe (Join-Path $crowdRoot 'Tests\LauncherDiagnosticsTests.cs') (Join-Path $PSScriptRoot 'LaunchDiagnostics.cs')
        if($LASTEXITCODE -ne 0){throw 'Launcher diagnostics tests build failed'}
        & '.\LauncherDiagnosticsTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Launcher diagnostics tests failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /platform:x64 /warnaserror /out:ManagedRuntimeLoaderTests.exe (Join-Path $crowdRoot 'Tests\ManagedRuntimeLoaderTests.cs') (Join-Path $PSScriptRoot 'ManagedRuntimeLoader.cs') (Join-Path $PSScriptRoot 'LaunchDiagnostics.cs')
        if($LASTEXITCODE -ne 0){throw 'Direct loader tests build failed'}
        & '.\ManagedRuntimeLoaderTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Direct loader tests failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /out:SettingsTests.exe (Join-Path $crowdRoot 'Tests\SettingsTests.cs') (Join-Path $PSScriptRoot 'PluginSettings.cs')
        if($LASTEXITCODE -ne 0){throw 'Settings tests build failed'}
        & '.\SettingsTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Settings tests failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /out:PluginStorageTests.exe (Join-Path $crowdRoot 'Tests\PluginStorageTests.cs') (Join-Path $PSScriptRoot 'PluginSettings.cs') (Join-Path $PSScriptRoot 'PluginStorage.cs')
        if($LASTEXITCODE -ne 0){throw 'Storage tests build failed'}
        & '.\PluginStorageTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Storage tests failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /out:RuntimeStorageTests.exe (Join-Path $crowdRoot 'Tests\RuntimeStorageTests.cs') (Join-Path $PSScriptRoot 'PluginSettings.cs') (Join-Path $PSScriptRoot 'PluginStorage.cs')
        if($LASTEXITCODE -ne 0){throw 'Runtime storage tests build failed'}
        & '.\RuntimeStorageTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Runtime storage tests failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /target:winexe /platform:x64 /reference:System.Windows.Forms.dll /reference:System.Drawing.dll /reference:System.Runtime.Serialization.dll ('/resource:'+$crowdReportFile+',Dga.CompatibilitySites') /out:DgaDiagnostics.exe (Join-Path $PSScriptRoot 'DgaDiagnostics.cs') (Join-Path $PSScriptRoot 'GameCompatibilityReport.cs')
        if($LASTEXITCODE -ne 0){throw 'Diagnostics window build failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /reference:System.Windows.Forms.dll /reference:System.Drawing.dll /reference:System.Runtime.Serialization.dll /main:DiagnosticWindowTests /out:DiagnosticWindowTests.exe (Join-Path $crowdRoot 'Tests\DiagnosticWindowTests.cs') (Join-Path $PSScriptRoot 'DgaDiagnostics.cs') (Join-Path $PSScriptRoot 'GameCompatibilityReport.cs')
        if($LASTEXITCODE -ne 0){throw 'Diagnostics window tests build failed'}
        & '.\DiagnosticWindowTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Diagnostics window tests failed'}
        & (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe') /nologo /platform:x64 /reference:System.Runtime.Serialization.dll /out:GameCompatibilityReportTests.exe (Join-Path $crowdRoot 'Tests\GameCompatibilityReportTests.cs') (Join-Path $PSScriptRoot 'GameCompatibilityReport.cs')
        if($LASTEXITCODE -ne 0){throw 'Compatibility report tests build failed'}
        & '.\GameCompatibilityReportTests.exe'
        if($LASTEXITCODE -ne 0){throw 'Compatibility report tests failed'}
        if(-not(Test-Path -LiteralPath 'DynamicGamedayAtmosphere.ini')){Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'DynamicGamedayAtmosphere.ini') -Destination 'DynamicGamedayAtmosphere.ini'}
    }
} finally {Pop-Location}
Write-Output ('Built '+$crowdOut)
