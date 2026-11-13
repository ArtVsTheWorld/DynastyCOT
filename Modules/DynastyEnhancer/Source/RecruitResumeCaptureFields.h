#pragma once
namespace dynasty::live {
inline std::set<std::string> resumeCaptureFields(const std::string& name){
    if(name=="Franchise")return {"LeagueID"};
    if(name=="SeasonInfo")return {"CurrentSeasonYear","CurrentYear","CurrentWeek","CurrentWeekType","CurrentStage","CurrentOffseasonStage"};
    if(name=="FranchiseServer_CareerFlow")return {"IsStageActive","IsStageAdvancing"};
    if(name=="Team")return {"TeamIndex","DisplayName","LongName","ShortName","CoachTalentEffects","SchoolPipelineInfluenceList"};
    if(name=="FranchiseUser")return {"Team","IsRetired"};
    if(name=="CoachTalentEffects")return {"Recruiting_ScoutingBoost_Start","Recruiting_BonusHours","Recruiting_InfluenceBoost_Start"};
    if(name=="int[]"||name=="SchoolPipelineInfluence[]")return {};
    if(name=="SchoolPipelineInfluence")return {"Pipeline","InfluenceLevel"};
    if(name=="Recruit")return {"Class","Player","AlternatePosition1","AlternatePosition2","NationalRank"};
    if(name=="Player")return {"Height","FirstName","LastName","HomePipeline","Position","TeamIndex","SchoolYear","RedshirtStatus","SeasonStats","CareerStats","PresentationId","ProspectStarRating","TraitDevelopment","PLYR_HOME_STATE","PLYR_HOME_TOWN","PLYR_BIRTHDATE","OverallRating","SpeedRating","AccelerationRating","AgilityRating","StrengthRating","AwarenessRating","BCVisionRating","ThrowPowerRating","ThrowAccuracyShortRating","ThrowAccuracyMidRating","ThrowAccuracyDeepRating","CatchingRating","CatchInTrafficRating","ShortRouteRunningRating","MediumRouteRunningRating","DeepRouteRunningRating","BreakTackleRating","CarryingRating","RunBlockRating","PassBlockRating","BlockSheddingRating","FinesseMovesRating","PowerMovesRating","TackleRating","PlayRecognitionRating","ManCoverageRating","ZoneCoverageRating","KickPowerRating","KickAccuracyRating","ChangeOfDirectionRating","ImpactBlockingRating","LongSnapRating","ReleaseRating"};
    if(name=="SeasonStats[]")return {};
    if((name.rfind("Season",0)==0||name.rfind("Career",0)==0)&&name.size()>=5&&name.substr(name.size()-5)=="Stats")return {"SEAS_YEAR","YEARBYYEARTEAMINDEX","GAMESPLAYED","DOWNSPLAYED","PASSATTEMPTS","PASSCOMPLETED","PASSYARDS","PASSTDS","PASSINTS","RUSHATTEMPTS","RUSHYARDS","RUSHTDS","RECEIVECATCHES","RECEIVEYARDS","RECEIVETDS","RECEIVEYARDSAFTER","RECEIVELONGEST","DEFTACKLES","ASSDEFTACKLES","DEFTACKLESFORLOSS","DLINESACKS","DLINEHALFSACK","DSECINTS","DEFPASSDEFLECTIONS","DLINEFORCEDFUMBLES","DLINEFUMBLERECOVERIES","OLINEPANCAKES","OLINESACKSALLOWED","KICKFGATTEMPTS","KICKFGMADE","KICKFGLONGEST","KICKEPATTEMPTS","KICKEPMADE","PUNTATTEMPTS","PUNTYARDS","PUNTIN20","KRETATTEMPTS","KRETYARDS","KRETTDS","PRETATTEMPTS","PRETYARDS","PRETTDS"};
    return {"__no_resume_fields__"};
}
// Background identity/history does not depend on optional coaching graphs.
inline std::set<std::string> resumeBackgroundFields(const std::string& name){
    auto fields=resumeCaptureFields(name);if(name=="Team"){fields.erase("CoachTalentEffects");fields.erase("SchoolPipelineInfluenceList");}return fields;
}
inline std::set<std::string> resumeIdentityFields(const std::string& name){
    if(name=="Franchise"||name=="SeasonInfo"||name=="FranchiseServer_CareerFlow")return resumeCaptureFields(name);
    return {"__no_notes_fields__"};
}
inline std::set<std::string> resumePipelineFields(const std::string& name){
    if(name=="Team")return {"TeamIndex","SchoolPipelineInfluenceList"};
    if(name=="FranchiseUser"||name=="SchoolPipelineInfluence"||name=="SchoolPipelineInfluence[]")return resumeCaptureFields(name);
    return resumeIdentityFields(name);
}
}
