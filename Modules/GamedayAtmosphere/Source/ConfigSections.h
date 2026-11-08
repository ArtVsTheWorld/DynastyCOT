#pragma once
#include <cwchar>
namespace attendance::config {
struct Route {const wchar_t* section;const wchar_t* key;};
inline Route route(const wchar_t* section,const wchar_t* key){
    if(!_wcsicmp(section,L"Atmosphere")&&(!_wcsicmp(key,L"SkillMoveReactions")||!_wcsicmp(key,L"StaggeredMajorCheers")||!_wcsicmp(key,L"PackedBigGameReactions")||!_wcsicmp(key,L"UpsetWatchAtmosphere")))return {L"Dynamic Gameday Soundscape",key};
    if(!_wcsicmp(section,L"Attendance")&&!_wcsicmp(key,L"Enabled"))return {L"Dynamic Gameday Attendance",L"Enabled"};
    if(!_wcsicmp(section,L"Attendance")&&!_wcsicmp(key,L"2020ModeEnabled"))return {L"Dynamic Gameday Attendance",L"2020ModeEnabled"};
    if(!_wcsicmp(section,L"Attendance")&&!_wcsicmp(key,L"InitialAttendanceEnabled"))return {L"Dynamic Gameday Attendance",L"InitialAttendanceEnabled"};
    if(!_wcsicmp(section,L"Attendance")&&!_wcsicmp(key,L"Profile"))return {L"Dynamic Gameday Attendance",L"Profile"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"FormulaStrength"))return {L"Dynamic Gameday Attendance",L"FormulaStrength"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"MaximumDeparturePerQuarter"))return {L"Dynamic Gameday Attendance",L"MaximumDeparturePerQuarter"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"MaximumDeparturePerTransition"))return {L"Dynamic Gameday Attendance",L"MaximumDeparturePerTransition"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"ComebackOriginalDeficit"))return {L"Dynamic Gameday Attendance",L"ComebackOriginalDeficit"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"ComebackPointsRecovered"))return {L"Dynamic Gameday Attendance",L"ComebackPointsRecovered"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"ComebackDeteriorationSeconds"))return {L"Dynamic Gameday Attendance",L"ComebackDeteriorationSeconds"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"TwoMinuteWarning"))return {L"Dynamic Gameday Attendance",L"TwoMinuteWarning"};
    if(!_wcsicmp(section,L"Retention")&&!_wcsicmp(key,L"LateScoreExtra"))return {L"Dynamic Gameday Attendance",L"LateScoreExtra"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"Enabled"))return {L"Dynamic Gameday Soundscape",L"LayeredAtmosphereEnabled"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"OpeningAtmosphere"))return {L"Dynamic Gameday Soundscape",L"OpeningAtmosphere"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"WinningEruptions"))return {L"Dynamic Gameday Soundscape",L"WinningEruptions"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"OvertimeAtmosphere"))return {L"Dynamic Gameday Soundscape",L"OvertimeAtmosphere"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"PlayCallAtmosphere"))return {L"Dynamic Gameday Soundscape",L"PlayCallAtmosphere"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"BigGameMinimumFullnessPercent"))return {L"Dynamic Gameday Soundscape",L"BigGameMinimumFullnessPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"PackedHouseBuildup"))return {L"Dynamic Gameday Soundscape",L"PackedHouseBuildup"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"DriveEscalation"))return {L"Dynamic Gameday Soundscape",L"DriveEscalation"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"ChantMaximumBoostPercent"))return {L"Dynamic Gameday Soundscape",L"ChantMaximumBoostPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"NativeShakeMinimumFullnessPercent"))return {L"Dynamic Gameday Broadcast",L"NativeShakeMinimumFullnessPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"CameraShake"))return {L"Dynamic Gameday Broadcast",L"CameraShake"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"OrdinaryShakeStrengthPercent"))return {L"Dynamic Gameday Broadcast",L"OrdinaryShakeStrengthPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"CriticalShakeStrengthPercent"))return {L"Dynamic Gameday Broadcast",L"CriticalShakeStrengthPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"TouchdownShakeStrengthPercent"))return {L"Dynamic Gameday Broadcast",L"TouchdownShakeStrengthPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"EruptionShakeStrengthPercent"))return {L"Dynamic Gameday Broadcast",L"EruptionShakeStrengthPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"TouchdownShakeImpactPercent"))return {L"Dynamic Gameday Broadcast",L"TouchdownShakeImpactPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"EruptionShakeImpactPercent"))return {L"Dynamic Gameday Broadcast",L"EruptionShakeImpactPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"TouchdownShakeDurationSeconds"))return {L"Dynamic Gameday Broadcast",L"TouchdownShakeDurationSeconds"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"EruptionShakeDurationSeconds"))return {L"Dynamic Gameday Broadcast",L"EruptionShakeDurationSeconds"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"MaximumBoostPercent"))return {L"Dynamic Gameday Soundscape",L"MaximumBoostPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"HighLeverageBoostPercent"))return {L"Dynamic Gameday Soundscape",L"HighLeverageBoostPercent"};
    if(!_wcsicmp(section,L"Atmosphere")&&!_wcsicmp(key,L"EruptionBoostPercent"))return {L"Dynamic Gameday Soundscape",L"EruptionBoostPercent"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"Enabled"))return {L"Dynamic Gameday Attendance",L"ArrivalsEnabled"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"StartPercent"))return {L"Dynamic Gameday Attendance",L"StartPercent"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"LateSchoolStartPercent"))return {L"Dynamic Gameday Attendance",L"LateSchoolStartPercent"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"ModerateStartPercent"))return {L"Dynamic Gameday Attendance",L"ModerateStartPercent"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"FinishPercentOfQuarter"))return {L"Dynamic Gameday Attendance",L"FinishPercentOfQuarter"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"LateSchoolFinishPercentOfQuarter"))return {L"Dynamic Gameday Attendance",L"LateSchoolFinishPercentOfQuarter"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"ModerateFinishPercentOfQuarter"))return {L"Dynamic Gameday Attendance",L"ModerateFinishPercentOfQuarter"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"HalftimeSeatBreakStrength"))return {L"Dynamic Gameday Attendance",L"HalftimeSeatBreakStrength"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"BigGameBonus"))return {L"Dynamic Gameday Attendance",L"BigGameBonus"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"BadWeatherPenalty"))return {L"Dynamic Gameday Attendance",L"BadWeatherPenalty"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"PlayoffStartPercent"))return {L"Dynamic Gameday Attendance",L"PlayoffStartPercent"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"ChampionshipStartPercent"))return {L"Dynamic Gameday Attendance",L"ChampionshipStartPercent"};
    if(!_wcsicmp(section,L"Arrivals")&&!_wcsicmp(key,L"FcsOpponentPenalty"))return {L"Dynamic Gameday Attendance",L"FcsOpponentPenalty"};
    if(!_wcsicmp(section,L"Audio")&&!_wcsicmp(key,L"Enabled"))return {L"Dynamic Gameday Soundscape",L"Enabled"};
    if(!_wcsicmp(section,L"Audio")&&!_wcsicmp(key,L"ReductionStartPercent"))return {L"Dynamic Gameday Soundscape",L"ReductionStartPercent"};
    if(!_wcsicmp(section,L"Audio")&&!_wcsicmp(key,L"NativeQuietingStrengthPercent"))return {L"Dynamic Gameday Soundscape",L"NativeQuietingStrengthPercent"};
    if(!_wcsicmp(section,L"Audio")&&!_wcsicmp(key,L"CrowdVolumeReductionPercent"))return {L"Dynamic Gameday Soundscape",L"CrowdVolumeReductionPercent"};
    if(!_wcsicmp(section,L"Audio")&&!_wcsicmp(key,L"MaximumReductionPercent"))return {L"Dynamic Gameday Soundscape",L"MaximumReductionPercent"};
    if(!_wcsicmp(section,L"Replays")&&!_wcsicmp(key,L"Enabled"))return {L"Dynamic Gameday Broadcast",L"ExtraReplaysEnabled"};
    if(!_wcsicmp(section,L"Replays")&&!_wcsicmp(key,L"InsetAfterRoutinePlays"))return {L"Dynamic Gameday Broadcast",L"InsetAfterRoutinePlays"};
    if(!_wcsicmp(section,L"Replays")&&!_wcsicmp(key,L"FullScreenBigPlays"))return {L"Dynamic Gameday Broadcast",L"FullScreenBigPlays"};
    if(!_wcsicmp(section,L"Replays")&&!_wcsicmp(key,L"FullScreenMinimumYards"))return {L"Dynamic Gameday Broadcast",L"FullScreenMinimumYards"};
    if(!_wcsicmp(section,L"Replays")&&!_wcsicmp(key,L"DefensiveHighlights"))return {L"Dynamic Gameday Broadcast",L"DefensiveHighlights"};
    if(!_wcsicmp(section,L"Attendance")&&!_wcsicmp(key,L"AttendanceProfile"))return {L"Dynamic Gameday Attendance",L"AttendanceProfile"};
    return {section,key};
}
}
