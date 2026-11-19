// Generated class/method contracts. No asset offsets or native bodies.
#pragma once
namespace recruitingfog::uidiscovery {
inline std::vector<Contract> configureAdapter(const std::string& bank,notesui::Runtime& runtime,bool dependency=false){
auto programs=runtime.discoveryPrograms();std::vector<Contract> contracts;
if(bank=="card.0"){contracts={{"Draw",{"_global","art_control","Object","module","spreadsheet","PlayerModule","cafe","enum"},625,6603195922241316ull,229},{"Draw",{"_global","art_control","Object","group","AttributesItem","widget","core","Widget"},141,17709153824899192835ull,122},{"Draw",{"_global","art_control","Object","module","spreadsheet","AttributesInfoSlot","cafe","enum"},124,6959257038863157567ull,111},{"Draw",{"_global","art_control","Object","module","spreadsheet","SkillGroupsInfoSlot","cafe","enum"},107,2667183236978171701ull,89},{"Draw",{"_global","art_control","Object","group","RatingValueItem","cafe","enum","InvalidationType"},184,11939708315136182041ull,135},{"SetValue",{"_global","art_control","Object","group","RatingValueItem","cafe","enum","InvalidationType"},184,11939708315136182041ull,97},{"SetValue",{"_global","art_control","Object","group","OVRItem","cafe","enum","InvalidationType"},122,13349314879580104119ull,69},{"_OnPageModuleChanged",{"_global","art_control","Object","module","spreadsheet","PlayerModule","cafe","enum"},625,6603195922241316ull,391}};
}
if(bank=="card.1"){contracts={{"_OnPlayerModuleUpdate",{"_global","art_screen","Object","manageplayer","ManagePlayerArt","mScreenResolution","mSizeStyleguide","art_styleguide"},480,11448716425025407191ull,275},{"HandleArtEvent",{"_global","art_screen","Object","manageplayer","ManagePlayerArt","mScreenResolution","mSizeStyleguide","art_styleguide"},480,11448716425025407191ull,400}};
for(auto& q:programs.at(1).nestedDefinitions)if(q.offset==144){q.semantic=true;q.nameText="";q.arguments={{255,"type"},{255,"params"}};}
for(auto& q:programs.at(1).nestedDefinitions)if(q.offset==264){q.semantic=true;q.nameText="";q.arguments={{255,"type"},{255,"params"}};}
for(auto& q:programs.at(1).nestedDefinitions)if(q.offset==384){q.semantic=true;q.nameText="";q.arguments={};}
}
if(bank=="card.2"){contracts={{"GetTemplate",{"_global","art_screen","Object","manageplayer","Skills","mScreenResolution","madden","screen"},125,6743042453220319225ull,27}};
}
if(bank=="card.3"){contracts={{"_SkillItemSetup",{"_global","logic_cfm","Object","model","manageplayer","Skills","fb_logic_com","BaseModel"},313,4559421617808188005ull,92},{"_UpdateSkillLevel",{"_global","logic_cfm","Object","model","manageplayer","Skills","fb_logic_com","BaseModel"},313,4559421617808188005ull,273},{"_UpdateSubskillsList",{"_global","logic_cfm","Object","model","manageplayer","Skills","fb_logic_com","BaseModel"},313,4559421617808188005ull,279}};
}
if(bank=="card.4"){contracts={{"_SetPlayersData",{"_global","art_control","Object","paneltile","TeamFormation","cafe","enum","InvalidationType"},286,5404633960517861759ull,106},{"_SetPlayerOvrParams",{"_global","art_screen","Object","panel","career","dynastyhubinfopanels","InfoPanelPlayerCompare","BaseInfoPanel"},217,18416570442040802502ull,95},{"_GetOvrParams",{"_global","art_screen","Object","panel","career","dynastyhubinfopanels","InfoPanelPlayersToWatch","DIVIDER_HEIGHT"},163,1266095928552644907ull,95},{"_StatValue",{"_global","art_screen","Object","panel","career","dynastyhubinfopanels","InfoPanelPlayerCompare","BaseInfoPanel"},217,18416570442040802502ull,77}};
}
if(bank=="card.5"){contracts={{"Draw",{"_global","art_control","Object","slider","GenericSlider","STEPS_PER_SPEED","cafe","enum"},321,3980275211647587210ull,223}};
}
if(bank=="card.6"){contracts={{"Draw",{"_global","art_screen","Object","playcall","main","PlayerRatingsPanel","widget","core"},123,1758487922010183909ull,105},{"Draw",{"_global","art_screen","Object","playcall","main","KeyPlayerItem","widget","core"},107,11786142425738284844ull,93},{"Draw",{"_global","art_screen","Object","playcall","main","PlaycallButton","widget","core"},386,1135574536652364527ull,273},{"Draw",{"_global","art_screen","Object","playcall","main","PlayerNameButton","widget","core"},129,5504390375677614502ull,116},{"Draw",{"_global","art_screen","Object","playcall","main","DepthChartPanelListItem","DEFAULT_HEIGHT","DEFAULT_WIDTH"},112,9309594208288281643ull,99},{"Draw",{"_global","art_control","Object","list","listItem","preplayHints","WearTearSubListItem","cafe"},160,6641644223907251368ull,125}};
}
if(bank=="card.7"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","MySchoolRiskListItem","cafe","enum","InvalidationType"},73,15491881949990271931ull,57}};
}
if(bank=="card.8"){contracts={{"_UpdatePlayer",{"_global","art_screen","Object","playbook","FormSubModuleTile","cafe","enum","InvalidationType"},240,17839311105999001167ull,181},{"Draw",{"_global","art_screen","Object","playbook","FormSubListItem","cafe","enum","InvalidationType"},93,12314546128768030583ull,55},{"_LookUnfocused",{"_global","art_screen","Object","playbook","FormSubListItem","cafe","enum","InvalidationType"},93,12314546128768030583ull,88},{"Draw",{"_global","art_screen","Object","playbook","FormSubPlayerTile","CACHED_ITEMDATA_SETUPCALLBACK","AttributesItemSetupCallback","cafe"},258,2214690460053445807ull,180}};
}
if(bank=="card.9"){contracts={{"Draw",{"_global","art_control","Object","group","OVRItem","cafe","enum","InvalidationType"},122,13349314879580104119ull,82}};
}
if(bank=="card.10"){contracts={{"Draw",{"_global","art_control","Object","programpoints","RosterNILRowItem","ARROW_DOWN_ROTATION","ARROW_SIZE","ARROW_OFFSET"},204,71726429872380297ull,155},{"_LookUnfocused",{"_global","art_control","Object","programpoints","RosterNILRowItem","ARROW_DOWN_ROTATION","ARROW_SIZE","ARROW_OFFSET"},204,71726429872380297ull,194},{"Draw",{"_global","art_control","Object","programpoints","RosterNILRowItem","ARROW_DOWN_ROTATION","ARROW_SIZE","ARROW_OFFSET"},204,71726429872380297ull,155}};
for(auto& q:programs.at(2).sourcePointers)if(q.offset==296){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==312){q.byText=true;q.text=",";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==328){q.byText=true;q.text="split";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==344){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==360){q.byText=true;q.text="join";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==376){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==432){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==448){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==472){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==488){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==504){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==528){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==552){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==600){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==616){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==640){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==656){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==672){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==696){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==720){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==752){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==768){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==784){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==800){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==816){q.byText=true;q.text="$";}
}
if(bank=="card.11"){contracts={{"Draw",{"_global","art_control","Object","programpoints","NILPlayerListItem","FOCUS_ANIM_NAME","CollectionTemplate Focus","UNFOCUS_ANIM_NAME"},177,9376056795344653044ull,153}};
}
if(bank=="card.12"){contracts={{"Draw",{"_global","art_control","Object","gameplanning","WearAndTearManagementRowItem","DEFAULT_HEIGHT","DEFAULT_WIDTH","INJURY_DEFAULT_X"},220,3789960237258241294ull,176},{"_LookUnfocused",{"_global","art_control","Object","gameplanning","WearAndTearManagementRowItem","DEFAULT_HEIGHT","DEFAULT_WIDTH","INJURY_DEFAULT_X"},220,3789960237258241294ull,216}};
}
if(bank=="card.13"){contracts={{"Draw",{"_global","art_control","Object","programpoints","RosterNILRowItem","ARROW_DOWN_ROTATION","ARROW_SIZE","ARROW_OFFSET"},204,71726429872380297ull,155},{"_LookUnfocused",{"_global","art_control","Object","programpoints","RosterNILRowItem","ARROW_DOWN_ROTATION","ARROW_SIZE","ARROW_OFFSET"},204,71726429872380297ull,194}};
}
if(bank=="card.14"){contracts={{"_InitializeComponents",{"_global","art_screen","Object","playcall","panel","KickerRange","mScreenResolution","madden"},109,2990894045797648014ull,12},{"_SetRange",{"_global","art_screen","Object","playcall","panel","KickerRange","mScreenResolution","madden"},109,2990894045797648014ull,89}};
}
if(bank=="card.15"){contracts={{"_UpdatePanelData",{"_global","art_screen","Object","playcall","panel","IndividualPositionStats","mScreenResolution","DEFAULT_WIDTH"},121,398182647259853759ull,101}};
}
if(bank=="card.16"){contracts={{"_UpdatePanelData",{"_global","art_screen","Object","playcall","panel","PlayerVsPlayer","mScreenResolution","madden"},130,14801154351755105791ull,92}};
}
if(bank=="card.17"){contracts={{"_InitializeComponents",{"_global","art_screen","Object","playcall","panel","ImpactPlayerDepth","mScreenResolution","DEFAULT_WIDTH"},97,595333355209474010ull,14}};
}
if(bank=="card.18"){contracts={{"_AddPosGroupItems",{"_global","art_screen","Object","playcall","panel","TeamPositionGroups","madden","screen"},204,15458509302393824946ull,113},{"_AddPosGroupKeyStatItems",{"_global","art_screen","Object","playcall","panel","TeamPositionGroups","madden","screen"},204,15458509302393824946ull,114}};
}
if(bank=="card.19"){contracts={{"_PopulateData",{"_global","art_screen","Object","playcall","panel","Trench","mScreenResolution","madden"},200,2882151831727375367ull,113}};
}
if(bank=="card.20"){contracts={{"UpdateSpreadsheetActionButtonVisibility",{"_global","Madden_ScrInjuryReport_Class","InjuryReport","Franchise","ScreenFactory","Spawn","mFranchiseInjuryReportAdapter","Madden_Franchise_InjuryReport"},353,10260536570957147598ull,93}};
}
if(bank=="card.21"){contracts={{"SetLabel",{"_global","art_control","Object","list","SwappableAndTextItem","cafe","enum","InvalidationType"},117,15126576985815005ull,83},{"Draw",{"_global","art_control","Object","list","SwappableAndTextItem","cafe","enum","InvalidationType"},117,15126576985815005ull,86}};
}
if(bank=="card.22"){contracts={{"Draw",{"_global","art_control","Object","module","spreadsheet","InfoSlotsItemWidget","cafe","enum"},150,10157023899431720268ull,123}};
}
if(bank=="card.23"){contracts={{"SetData",{"_global","art_control","Object","spreadsheet","ProgramPointsCell","madden","component","SpreadsheetCell"},45,2833417680316908504ull,32}};
}
if(bank=="card.24"){contracts={{"Draw",{"_global","art_control","Object","module","spreadsheet","TeamModule","cafe","enum"},192,1297239276679392684ull,128}};
}
if(bank=="card.25"){contracts={{"_SetAttributesParams",{"_global","art_screen","Object","panel","career","dynastyhubinfopanels","InfoPanelPOTW","BaseInfoPanel"},141,7225175514583079477ull,92},{"Init",{"_global","art_screen","Object","panel","career","dynastyhubinfopanels","InfoPanelPregameStats","TITLE_INFO_INDEX"},151,2568892455995420355ull,29},{"BindWidget",{"_global","logic_cfm","Object","model","dynastyhubinfopanels","InfoPanelPregameTeamStats","fb_logic_com","BaseModel"},126,8485668168155727896ull,37}};
}
if(bank=="card.26"){contracts={{"Draw",{"_global","art_control","Object","module","spreadsheet","DealBreakerInfoSlot","cafe","enum"},113,14629270998624711982ull,87}};
}
if(bank=="card.27"){contracts={{"_UpdateStatBoxes",{"_global","madden","Object","screen","teamselect","TeamSelect","BaseScreen","prototype"},947,8319870483240972716ull,441}};
}
if(bank=="card.28"){contracts={{"_GetTeamModule",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","PlayersLeavingStrategy","UcmSpreadsheetModuleStrategy"},368,10970186188470136322ull,49}};
}
if(bank=="card.29"){contracts={{"_GetTeamModule",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","PlayersLeavingStrategy","UcmSpreadsheetModuleStrategy"},368,10970186188470136322ull,49}};
}
if(bank=="card.30"){contracts={{"GetRowData",{"_global","madden","Object","provider","SpreadsheetProvider","prototype","ion","component"},232,2736444976733081140ull,100}};
}
if(bank=="card.31"){contracts={{"SetExtraInfo",{"_global","art_screen","Object","infocenter","CareerInfoModule","DEFAULT_BAR_WIDTH","DEFAULT_HEIGHT","RESIZE_WIDTH"},362,6035713643625722744ull,99}};
}
if(bank=="card.32"){contracts={{"_RefreshStats",{"_global","logic_cfm","Object","model","recruitinghub","HubScreen","BUMPER_INPUT_DELAY","RECRUITING HUB SCREEN BUMPER INPUT DELAY"},190,11148305110749509497ull,60}};
}
if(bank=="card.33"){contracts={{"_UpdateNumber",{"_global","art_control","Object","programpoints","ProgramBudgetMySchoolGradeRow","STYLE_BUDGET","STYLE_GRADE_AND_PROGRAM_PTS","STYLE_GRADE"},207,15213598443468387917ull,132},{"Draw",{"_global","art_control","Object","programpoints","ProgramBudgetMySchoolGradeRow","STYLE_BUDGET","STYLE_GRADE_AND_PROGRAM_PTS","STYLE_GRADE"},207,15213598443468387917ull,168},{"_SetupStageObjects",{"_global","art_popup","Object","recruitinghub","OfferScholarshipPopup","mScreenResolution","GetScreenResolution","FONT_STYLEGUIDE"},450,5600429906870955295ull,53},{"_OnOfferChanged",{"_global","art_popup","Object","recruitinghub","OfferScholarshipPopup","mScreenResolution","GetScreenResolution","FONT_STYLEGUIDE"},450,5600429906870955295ull,360},{"Draw",{"_global","art_control","Object","coachcarousel","SchoolInfoModule","DEFAULT_HEIGHT","DEFAULT_WIDTH","DIVIDER_HORIZONTAL_WIDTH"},247,5702872876468454175ull,192},{"Draw",{"_global","art_control","Object","coachcarousel","ProgramPointsModule","GRAPH_SIZE","MAX_BUDGET_ITEMS","BUDGET_BG_PADDING"},242,495378438371626668ull,134}};
}
if(bank=="card.34"){contracts={{"_SetLabel",{"_global","fb_art_utility","Object","TextUtil","prototype","Increment","mTargetHash","HaltIncrement"},104,10002460637964771739ull,14}};
}
if(bank=="card.35"){contracts={{"Draw",{"_global","art_control","Object","programpoints","PointsAllocation","ICON_SIZE","DEFAULT_HEIGHT","DEFAULT_WIDTH"},123,13884484899730943716ull,90},{"_SetGraphLabelData",{"_global","art_control","Object","programpoints","ProgramPointsPieGraph","cafe","enum","InvalidationType"},161,3625403973432693533ull,74}};
}
if(bank=="card.36"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","SummaryPanelListItem","cafe","enum","InvalidationType"},160,11459641417632374385ull,144}};
}
if(bank=="card.37"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,141},{"_AddCurrentValue",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,160},{"_SetCurrentValueToMax",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,161},{"_SetCurrentValueToMin",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,162},{"_SetCurrentValueToDefault",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,163},{"HandleInput",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,151}};
}
if(bank=="card.38"){contracts={{"SetData",{"_global","art_control","Object","spreadsheet","ProgramPointsCell","madden","component","SpreadsheetCell"},45,2833417680316908504ull,32},{"_UpdateNumber",{"_global","art_control","Object","programpoints","ProgramBudgetMySchoolGradeRow","STYLE_BUDGET","STYLE_GRADE_AND_PROGRAM_PTS","STYLE_GRADE"},207,15213598443468387917ull,132},{"Draw",{"_global","art_control","Object","programpoints","ProgramBudgetMySchoolGradeRow","STYLE_BUDGET","STYLE_GRADE_AND_PROGRAM_PTS","STYLE_GRADE"},207,15213598443468387917ull,168},{"_SetupStageObjects",{"_global","art_popup","Object","recruitinghub","OfferScholarshipPopup","mScreenResolution","GetScreenResolution","FONT_STYLEGUIDE"},450,5600429906870955295ull,53},{"_OnOfferChanged",{"_global","art_popup","Object","recruitinghub","OfferScholarshipPopup","mScreenResolution","GetScreenResolution","FONT_STYLEGUIDE"},450,5600429906870955295ull,360},{"Draw",{"_global","art_control","Object","coachcarousel","SchoolInfoModule","DEFAULT_HEIGHT","DEFAULT_WIDTH","DIVIDER_HORIZONTAL_WIDTH"},247,5702872876468454175ull,192},{"Draw",{"_global","art_control","Object","coachcarousel","ProgramPointsModule","GRAPH_SIZE","MAX_BUDGET_ITEMS","BUDGET_BG_PADDING"},242,495378438371626668ull,134},{"Draw",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,141},{"_AddCurrentValue",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,160},{"_SetCurrentValueToMax",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,161},{"_SetCurrentValueToMin",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,162},{"_SetCurrentValueToDefault",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,163},{"HandleInput",{"_global","art_control","Object","recruitinghub","ScholarshipToggle","STYLEGUIDE","art_styleguide","ListItems"},188,5424993852732571140ull,151}};
for(auto& q:programs.at(0).sourcePointers)if(q.offset==16){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==32){q.byText=true;q.text=",";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==48){q.byText=true;q.text="split";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==64){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==80){q.byText=true;q.text="join";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==96){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==152){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==168){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==192){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==208){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==224){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==248){q.byText=true;q.text=".";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==272){q.byText=true;q.text="M";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==320){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==336){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==360){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==376){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==392){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==416){q.byText=true;q.text=".";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==440){q.byText=true;q.text="K";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==472){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==488){q.byText=true;q.text="utility";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==504){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==520){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==536){q.byText=true;q.text="$";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==32){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==48){q.byText=true;q.text=",";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==64){q.byText=true;q.text="split";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==80){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==96){q.byText=true;q.text="join";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==112){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==168){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==184){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==208){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==224){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==240){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==264){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==288){q.byText=true;q.text="M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==336){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==352){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==376){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==392){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==408){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==432){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==456){q.byText=true;q.text="K";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==488){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==504){q.byText=true;q.text="utility";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==520){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==536){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==552){q.byText=true;q.text="$";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==584){q.byText=true;q.text="SetLabel";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==288){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==304){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==328){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==344){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==360){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==384){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==408){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==456){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==472){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==496){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==512){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==528){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==552){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==576){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==608){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==624){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==640){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==656){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==672){q.byText=true;q.text="$";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==880){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==896){q.byText=true;q.text=",";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==912){q.byText=true;q.text="split";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==928){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==944){q.byText=true;q.text="join";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==960){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1016){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1032){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1056){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1072){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1088){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1112){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1136){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1184){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1200){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1224){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1240){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1256){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1280){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1304){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1336){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1352){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1368){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1384){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1400){q.byText=true;q.text="$";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1456){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1472){q.byText=true;q.text=",";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1488){q.byText=true;q.text="split";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1504){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1520){q.byText=true;q.text="join";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1536){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1592){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1608){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1632){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1648){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1664){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1688){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1712){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1760){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1776){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1800){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1816){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1832){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1856){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1880){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1912){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1928){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1944){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1960){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==1976){q.byText=true;q.text="$";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2320){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2336){q.byText=true;q.text=",";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2352){q.byText=true;q.text="split";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2368){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2384){q.byText=true;q.text="join";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2400){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2456){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2472){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2496){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2512){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2528){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2552){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2576){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2624){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2640){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2664){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2680){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2696){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2720){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2744){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2776){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2792){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2808){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2824){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==2840){q.byText=true;q.text="$";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5312){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5328){q.byText=true;q.text=",";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5344){q.byText=true;q.text="split";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5360){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5376){q.byText=true;q.text="join";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5392){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5448){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5464){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5488){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5504){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5520){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5544){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5568){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5616){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5632){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5656){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5672){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5688){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5712){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5736){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5768){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5784){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5800){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5816){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5832){q.byText=true;q.text="$";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5912){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5928){q.byText=true;q.text=",";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5944){q.byText=true;q.text="split";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5960){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5976){q.byText=true;q.text="join";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==5992){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6048){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6064){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6088){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6104){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6120){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6144){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6168){q.byText=true;q.text="M";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6216){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6232){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6256){q.byText=true;q.text="Math";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6272){q.byText=true;q.text="floor";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6288){q.byText=true;q.text="";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6312){q.byText=true;q.text=".";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6336){q.byText=true;q.text="K";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6368){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6384){q.byText=true;q.text="utility";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6400){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6416){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(2).sourcePointers)if(q.offset==6432){q.byText=true;q.text="$";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1576){q.byText=true;q.text="Math";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1592){q.byText=true;q.text="floor";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1616){q.byText=true;q.text="Math";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1632){q.byText=true;q.text="floor";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1648){q.byText=true;q.text="";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1672){q.byText=true;q.text=".";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1696){q.byText=true;q.text="M";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1744){q.byText=true;q.text="Math";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1760){q.byText=true;q.text="floor";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1784){q.byText=true;q.text="Math";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1800){q.byText=true;q.text="floor";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1816){q.byText=true;q.text="";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1840){q.byText=true;q.text=".";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1864){q.byText=true;q.text="K";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1896){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1912){q.byText=true;q.text="utility";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1928){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1944){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==1960){q.byText=true;q.text="$";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2120){q.byText=true;q.text="$";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2152){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2168){q.byText=true;q.text="utility";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2184){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2200){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2960){q.byText=true;q.text="$";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==2984){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3000){q.byText=true;q.text="utility";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3016){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3032){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3160){q.byText=true;q.text="$";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3192){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3208){q.byText=true;q.text="utility";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3224){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3240){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3328){q.byText=true;q.text="$";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3360){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3376){q.byText=true;q.text="utility";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3392){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(3).sourcePointers)if(q.offset==3408){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==256){q.byText=true;q.text="$";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==280){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==296){q.byText=true;q.text="utility";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==312){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==328){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==648){q.byText=true;q.text="Math";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==664){q.byText=true;q.text="floor";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==688){q.byText=true;q.text="Math";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==704){q.byText=true;q.text="floor";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==720){q.byText=true;q.text="";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==744){q.byText=true;q.text=".";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==768){q.byText=true;q.text="M";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==816){q.byText=true;q.text="Math";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==832){q.byText=true;q.text="floor";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==856){q.byText=true;q.text="Math";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==872){q.byText=true;q.text="floor";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==888){q.byText=true;q.text="";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==912){q.byText=true;q.text=".";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==936){q.byText=true;q.text="K";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==968){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==984){q.byText=true;q.text="utility";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==1000){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==1016){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(4).sourcePointers)if(q.offset==1032){q.byText=true;q.text="$";}
for(auto& q:programs.at(5).sourcePointers)if(q.offset==104){q.byText=true;q.text="$";}
for(auto& q:programs.at(5).sourcePointers)if(q.offset==968){q.byText=true;q.text="$";}
for(auto& q:programs.at(5).sourcePointers)if(q.offset==1024){q.byText=true;q.text="$";}
for(auto& q:programs.at(6).sourcePointers)if(q.offset==304){q.byText=true;q.text="$";}
for(auto& q:programs.at(6).sourcePointers)if(q.offset==368){q.byText=true;q.text="$";}
for(auto& q:programs.at(7).sourcePointers)if(q.offset==72){q.byText=true;q.text="$";}
for(auto& q:programs.at(7).sourcePointers)if(q.offset==96){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(7).sourcePointers)if(q.offset==112){q.byText=true;q.text="utility";}
for(auto& q:programs.at(7).sourcePointers)if(q.offset==128){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(7).sourcePointers)if(q.offset==144){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(8).sourcePointers)if(q.offset==88){q.byText=true;q.text="$";}
for(auto& q:programs.at(8).sourcePointers)if(q.offset==112){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(8).sourcePointers)if(q.offset==128){q.byText=true;q.text="utility";}
for(auto& q:programs.at(8).sourcePointers)if(q.offset==144){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(8).sourcePointers)if(q.offset==160){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(9).sourcePointers)if(q.offset==32){q.byText=true;q.text="$";}
for(auto& q:programs.at(9).sourcePointers)if(q.offset==56){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(9).sourcePointers)if(q.offset==72){q.byText=true;q.text="utility";}
for(auto& q:programs.at(9).sourcePointers)if(q.offset==88){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(9).sourcePointers)if(q.offset==104){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(10).sourcePointers)if(q.offset==32){q.byText=true;q.text="$";}
for(auto& q:programs.at(10).sourcePointers)if(q.offset==56){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(10).sourcePointers)if(q.offset==72){q.byText=true;q.text="utility";}
for(auto& q:programs.at(10).sourcePointers)if(q.offset==88){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(10).sourcePointers)if(q.offset==104){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(11).sourcePointers)if(q.offset==32){q.byText=true;q.text="$";}
for(auto& q:programs.at(11).sourcePointers)if(q.offset==56){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(11).sourcePointers)if(q.offset==72){q.byText=true;q.text="utility";}
for(auto& q:programs.at(11).sourcePointers)if(q.offset==88){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(11).sourcePointers)if(q.offset==104){q.byText=true;q.text="FormatNumberString";}
}
if(bank=="card.39"){contracts={{"Draw",{"_global","art_control","Object","programpoints","PointsAllocation","ICON_SIZE","DEFAULT_HEIGHT","DEFAULT_WIDTH"},123,13884484899730943716ull,90},{"_SetGraphLabelData",{"_global","art_control","Object","programpoints","ProgramPointsPieGraph","cafe","enum","InvalidationType"},161,3625403973432693533ull,74}};
for(auto& q:programs.at(0).sourcePointers)if(q.offset==344){q.byText=true;q.text="$";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==8){q.byText=true;q.text="eoBoosterOpen";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==32){q.byText=true;q.text="DONOR HAPPINESS";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==48){q.byText=true;q.text="mcBudgetTitle";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==64){q.byText=true;q.text="SetLabel";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==88){q.byText=true;q.text="loc";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==104){q.byText=true;q.text="career";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==120){q.byText=true;q.text="ProgramPoints";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==136){q.byText=true;q.text="BLUEPRINT_BUDGET";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==152){q.byText=true;q.text="mcBudgetTitle";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==168){q.byText=true;q.text="SetLabel";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==352){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==368){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==392){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==408){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==424){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==448){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==472){q.byText=true;q.text="M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==520){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==536){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==560){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==576){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==592){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==616){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==640){q.byText=true;q.text="K";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==672){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==688){q.byText=true;q.text="utility";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==704){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==720){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==736){q.byText=true;q.text="$";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==768){q.byText=true;q.text="eoBoosterOpen";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==792){q.byText=true;q.text="eoHdr";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==816){q.byText=true;q.text="eoHdr";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==832){q.byText=true;q.text="happy100";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==848){q.byText=true;q.text="%";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==872){q.byText=true;q.text="--";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==888){q.byText=true;q.text="mcBudgetCurrency";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==904){q.byText=true;q.text="SetLabel";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1056){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1072){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1096){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1112){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1128){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1152){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1176){q.byText=true;q.text="M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1224){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1240){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1264){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1280){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1296){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1320){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1344){q.byText=true;q.text="K";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1376){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1392){q.byText=true;q.text="utility";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1408){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1424){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1440){q.byText=true;q.text="$";}
}
if(bank=="card.41"){contracts={{"_RefreshStats",{"_global","logic_cfm","Object","model","recruitinghub","HubScreen","BUMPER_INPUT_DELAY","RECRUITING HUB SCREEN BUMPER INPUT DELAY"},190,11148305110749509497ull,60},{"Draw",{"_global","art_control","Object","recruitinghub","PlayerListItem","cafe","enum","InvalidationType"},322,16900888385295740251ull,233},{"Draw",{"_global","art_control","Object","recruitinghub","TotalHours","cafe","enum","InvalidationType"},100,2671078463011149765ull,78}};
for(auto& q:programs.at(0).sourcePointers)if(q.offset==112){q.byText=true;q.text="$";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2872){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2888){q.byText=true;q.text=",";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2904){q.byText=true;q.text="split";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2920){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2936){q.byText=true;q.text="join";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2952){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3008){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3024){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3048){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3064){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3080){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3104){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3128){q.byText=true;q.text="M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3176){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3192){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3216){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3232){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3248){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3272){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3296){q.byText=true;q.text="K";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3328){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3344){q.byText=true;q.text="utility";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3360){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3376){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3392){q.byText=true;q.text="$";}
}
if(bank=="card.42"){contracts={{"Draw",{"_global","art_control","Object","programpoints","NILPlayerListItem","FOCUS_ANIM_NAME","CollectionTemplate Focus","UNFOCUS_ANIM_NAME"},177,9376056795344653044ull,153}};
for(auto& q:programs.at(0).sourcePointers)if(q.offset==56){q.byText=true;q.text="mData";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==72){q.byText=true;q.text="ExtraInfo";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==88){q.byText=true;q.text="IsCoach";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==128){q.byText=true;q.text="IsCoach";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==144){q.byText=true;q.text="mData";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==160){q.byText=true;q.text="ExtraInfo";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==176){q.byText=true;q.text="IsCoach";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==288){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==304){q.byText=true;q.text=",";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==320){q.byText=true;q.text="split";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==336){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==352){q.byText=true;q.text="join";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==368){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==424){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==440){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==464){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==480){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==496){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==520){q.byText=true;q.text=".";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==544){q.byText=true;q.text="M";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==592){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==608){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==632){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==648){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==664){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==688){q.byText=true;q.text=".";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==712){q.byText=true;q.text="K";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==744){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==760){q.byText=true;q.text="utility";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==776){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==792){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==808){q.byText=true;q.text="$";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==1608){q.byText=true;q.text="mData";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==1624){q.byText=true;q.text="ExtraInfo";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==1640){q.byText=true;q.text="EoNoLogo";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==1664){q.byText=true;q.text="mData";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==1680){q.byText=true;q.text="ExtraInfo";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==1696){q.byText=true;q.text="IsCoach";}
}
if(bank=="card.43"){contracts={{"Draw",{"_global","art_control","Object","myschoolreport","MySchoolGradePerWeek","cafe","enum","InvalidationType"},242,8441995312305626664ull,167}};
}
if(bank=="card.44"){contracts={{"_FillListPlayerData",{"_global","art_control","Object","coachcarousel","OverviewModule","cafe","enum","InvalidationType"},385,4640348547263028692ull,108}};
}
if(bank=="card.45"){contracts={{"_FillListPlayerData",{"_global","art_control","Object","coachcarousel","OverviewModule","cafe","enum","InvalidationType"},385,4640348547263028692ull,108}};
}
if(bank=="card.46"){contracts={{"GetSubscreenDataAtBoardLevel",{"_global","node_cfm","Object","recruitinghub","Navigation","FIRST_AVAILABLE_LAYER","SUBNODE_LAYER","mBoardTabScreenLayer"},203,14308827235194088065ull,167},{"Init",{"_global","logic_cfm","Object","model","recruitinghub","Board","mSortInfo","sortFunction"},333,14842884499352717657ull,52},{"_OnCategoryChanged",{"_global","logic_cfm","Object","model","recruitinghub","Board","mSortInfo","sortFunction"},333,14842884499352717657ull,231},{"_PopulateData",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,124},{"_InitializeFocus",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,99},{"_UpdateScoutButton",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,125},{"_OnScout",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,73},{"_OnRecruitUpdated",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,31},{"ModelHidden",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,127},{"Shutdown",{"_global","logic_cfm","Object","model","recruitinghub","board","Scouting","fb_logic_com"},258,14472779081281178557ull,247},{"Draw",{"_global","art_control","Object","recruitinghub","AttributeButton","cafe","enum","InvalidationType"},82,2377776789148036445ull,64},{"_LookFocused",{"_global","art_control","Object","recruitinghub","AttributeButton","cafe","enum","InvalidationType"},82,2377776789148036445ull,68},{"_LookUnfocused",{"_global","art_control","Object","recruitinghub","AttributeButton","cafe","enum","InvalidationType"},82,2377776789148036445ull,72}};
}
if(bank=="card.47"){contracts={{"Init",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,9},{"HandleInput",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,108},{"_ShowManagePlayerScreen",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,51}};
}
if(bank=="card.48"){contracts={{"Init",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,9},{"HandleInput",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,108},{"_ShowManagePlayerScreen",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,51}};
for(auto& q:programs.at(1).sourcePointers)if(q.offset==8){q.byText=true;q.text="KeyCode";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==24){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==40){q.byText=true;q.text="enum";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==56){q.byText=true;q.text="Input";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==72){q.byText=true;q.text="CODE_R1";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==96){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==120){q.byText=true;q.text="mPlayerModuleTeamLogoId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==136){q.byText=true;q.text="mUserTeamLogoId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==160){q.byText=true;q.text="mIsSpreadsheetEmpty";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==184){q.byText=true;q.text="mCurrentRowIndex";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==200){q.byText=true;q.text="mSpreadsheetDataProvider";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==216){q.byText=true;q.text="GetRowKey";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==248){q.byText=true;q.text="mRosterAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==264){q.byText=true;q.text="GetEntryInfo";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==304){q.byText=true;q.text="SR";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==320){q.byText=true;q.text="className";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==336){q.byText=true;q.text="indexOf";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==360){q.byText=true;q.text="eoTpName";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==376){q.byText=true;q.text="firstName";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==392){q.byText=true;q.text=" ";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==408){q.byText=true;q.text="lastName";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==424){q.byText=true;q.text="teamLogoId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==464){q.byText=true;q.text="mPlayerModuleTeamLogoId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==512){q.byText=true;q.text="jerseyNumber";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==536){q.byText=true;q.text="overall";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==560){q.byText=true;q.text="eoTpPick";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==576){q.byText=true;q.text="a";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==592){q.byText=true;q.text="b";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==608){q.byText=true;q.text="$8M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==632){q.byText=true;q.text="overall";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==656){q.byText=true;q.text="$5M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==688){q.byText=true;q.text="overall";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==712){q.byText=true;q.text="$3M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==744){q.byText=true;q.text="overall";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==768){q.byText=true;q.text="$2M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==800){q.byText=true;q.text="overall";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==824){q.byText=true;q.text="$1M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==856){q.byText=true;q.text="eoTpConfirm";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==928){q.byText=true;q.text="eoTpPick";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==952){q.byText=true;q.text="ion";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==968){q.byText=true;q.text="manager";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==984){q.byText=true;q.text="PopupManager";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1000){q.byText=true;q.text="INDEX_YES";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1024){q.byText=true;q.text="eoTpPick";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1040){q.byText=true;q.text="a";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1072){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1088){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1104){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1120){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1136){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1152){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1176){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1192){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1208){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1224){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1240){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1256){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1272){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1288){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1312){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1328){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1344){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1360){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1376){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1392){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1408){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1424){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1448){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1464){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1480){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1496){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1512){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1528){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1544){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1560){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1584){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1600){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1616){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1632){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1648){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1664){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1680){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1696){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1720){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1736){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1752){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1768){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1784){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1800){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1816){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1832){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1848){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1864){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1880){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1896){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1912){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1928){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1944){q.byText=true;q.text="eoTpPick";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1960){q.byText=true;q.text="b";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==1992){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2008){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2024){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2040){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2056){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2072){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2096){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2112){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2128){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2144){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2160){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2176){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2192){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2208){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2232){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2248){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2264){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2280){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2296){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2312){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2328){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2344){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2368){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2384){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2400){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2416){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2432){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2448){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2464){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2480){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2504){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2520){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2536){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2552){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2568){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2584){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2600){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2616){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2640){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2656){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2672){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2688){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2704){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2720){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2736){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2752){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2768){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2784){q.byText=true;q.text="madden";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2800){q.byText=true;q.text="ultimatecareer";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2816){q.byText=true;q.text="Adapters";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2832){q.byText=true;q.text="GetUcmAdapter";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2848){q.byText=true;q.text="ActionRequestRespond";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2864){q.byText=true;q.text="eoTpDone";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2880){q.byText=true;q.text="eoTpName";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2896){q.byText=true;q.text="eoTpPick";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2912){q.byText=true;q.text="_RefreshHints";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2928){q.byText=true;q.text="RefreshHints";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2944){q.byText=true;q.text="eoTpConfirm";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2960){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2976){q.byText=true;q.text="utility";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2992){q.byText=true;q.text="Delegate";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3008){q.byText=true;q.text="Create";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3024){q.byText=true;q.text="Send your boosters after ";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3040){q.byText=true;q.text="eoTpName";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3056){q.byText=true;q.text=" (";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3072){q.byText=true;q.text="position";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3088){q.byText=true;q.text=", ";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3104){q.byText=true;q.text="overall";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3120){q.byText=true;q.text=" OVR) for ";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3136){q.byText=true;q.text=" of program money? Tampered players are very likely to enter the transfer portal at season end. Tampering carries a risk of penalties if it gets out.";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3152){q.byText=true;q.text="TAMPERING PACKAGE";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3168){q.byText=true;q.text="POPUP_AUTO_PROGRESSION";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3184){q.byText=true;q.text="mContext";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3200){q.byText=true;q.text="ion";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3216){q.byText=true;q.text="manager";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3232){q.byText=true;q.text="PopupManagerService";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3248){q.byText=true;q.text="Get";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3264){q.byText=true;q.text="MakeYesNoPopup";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3280){q.byText=true;q.text="Open";}
for(auto& q:programs.at(1).nestedDefinitions)if(q.offset==872){q.semantic=true;q.nameText="";q.arguments={{3,"label"},{4,"index"},{5,"btn"}};}
}
if(bank=="card.49"){contracts={{"Shutdown",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,98}};
}
if(bank=="card.50"){contracts={{"Shutdown",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,98}};
}
if(bank=="card.51"){contracts={{"GetHints",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,123}};
}
if(bank=="card.52"){contracts={{"Draw",{"_global","art_control","Object","module","spreadsheet","TeamModuleHorizontal","cafe","enum"},240,3365135062889920035ull,171}};
}
if(bank=="card.53"){contracts={{"_RefreshDataComplete",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,269}};
}
if(bank=="card.54"){contracts={{"_RefreshDataComplete",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,269}};
}
if(bank=="card.55"){contracts={{"GetHints",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,123}};
for(auto& q:programs.at(0).sourcePointers)if(q.offset==160){q.byText=true;q.text="eoUpId";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==184){q.byText=true;q.text="mPlayerModuleTeamLogoId";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==200){q.byText=true;q.text="mUserTeamLogoId";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==224){q.byText=true;q.text="mIsSpreadsheetEmpty";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==248){q.byText=true;q.text="Tamper ($1M-$8M)";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==280){q.byText=true;q.text="eoTpDone";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==304){q.byText=true;q.text="Tampering: ";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==320){q.byText=true;q.text="eoTpDone";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==336){q.byText=true;q.text=" (lands next week)";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==376){q.byText=true;q.text="mCurrentRowIndex";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==392){q.byText=true;q.text="mSpreadsheetDataProvider";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==408){q.byText=true;q.text="GetRowKey";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==440){q.byText=true;q.text="mRosterAdapter";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==456){q.byText=true;q.text="GetEntryInfo";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==496){q.byText=true;q.text="SR";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==512){q.byText=true;q.text="className";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==528){q.byText=true;q.text="indexOf";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==552){q.byText=true;q.text="Seniors can't transfer";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==584){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==600){q.byText=true;q.text="enum";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==616){q.byText=true;q.text="Input";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==632){q.byText=true;q.text="CODE_R1";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==648){q.byText=true;q.text="ion";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==664){q.byText=true;q.text="struct";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==680){q.byText=true;q.text="HintData";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==696){q.byText=true;q.text="push";}
}
if(bank=="card.56"){contracts={{"_RowChanged",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,138}};
}
if(bank=="card.57"){contracts={{"_RowChanged",{"_global","madden","Object","ultimatecareer","screen","spreadsheets","RosterStrategy","UcmSpreadsheetModuleStrategy"},328,13254302549808637832ull,138}};
for(auto& q:programs.at(0).sourcePointers)if(q.offset==544){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==560){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==584){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==600){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==616){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==640){q.byText=true;q.text=".";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==664){q.byText=true;q.text="M";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==712){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==728){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==752){q.byText=true;q.text="Math";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==768){q.byText=true;q.text="floor";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==784){q.byText=true;q.text="";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==808){q.byText=true;q.text=".";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==832){q.byText=true;q.text="K";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==864){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==880){q.byText=true;q.text="utility";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==896){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==912){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(0).sourcePointers)if(q.offset==928){q.byText=true;q.text="$";}
}
if(bank=="card.58"){contracts={{"Draw",{"_global","art_control","Object","toggle","GenericToggle","cafe","enum","InvalidationType"},416,5539039109163510029ull,292},{"SetLabel",{"_global","art_control","Object","toggle","DefaultToggleItem","widget","core","Widget"},101,8620186857033482888ull,75}};
}
if(bank=="card.59"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","PlayerListItem","cafe","enum","InvalidationType"},322,16900888385295740251ull,233}};
}
if(bank=="card.60"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","TotalHours","cafe","enum","InvalidationType"},100,2671078463011149765ull,78}};
}
if(bank=="card.61"){contracts={{"GetRowData",{"_global","madden","Object","ultimatecareer","provider","PositionChangesProvider","UcmSpreadsheetProvider","prototype"},28,14458108369670695404ull,14}};
}
if(bank=="card.62"){contracts={{"_OnSpreadsheetRowChange",{"_global","Madden_ScrDepthChartSelectPlayer_Class","DepthChartSelectPlayer","Franchise","ScreenFactory","Spawn","mDepthChartAdapter","Madden_Franchise_DepthChartSelectPlayer"},510,15491260051793869076ull,341}};
}
if(bank=="card.63"){contracts={{"Draw",{"_global","art_control","Object","weeklygameplan","SituationItem","cafe","enum","InvalidationType"},73,14446405058216107307ull,63}};
}
if(bank=="card.64"){contracts={{"_SetStatsData",{"_global","art_screen","Object","weeklygameplan","TeamProfile","mScreenResolution","FONT_STYLEGUIDE","art_styleguide"},230,13781598353650661799ull,111}};
}
if(bank=="card.65"){contracts={{"_SetupPopupData",{"_global","fb_logic_cfm","Object","model","training","FocusTraining","MAX_PLAYERS","FOCUS_FAILED_POPUP"},158,6956725317441686010ull,71}};
}
if(bank=="grid.legacy"){contracts={{"GetCellColor",{"_global","madden","Object","provider","SpreadsheetProvider","prototype","ion","component"},232,2736444976733081140ull,160},{"GetColumnDefinitions",{"_global","madden","Object","provider","Cafe2SpreadsheetProvider","prototype","ion","component"},143,10252185081306720664ull,107},{"GetRowData",{"_global","madden","Object","provider","Cafe2SpreadsheetProvider","prototype","ion","component"},143,10252185081306720664ull,94},{"GetCellColor",{"_global","madden","Object","provider","Cafe2SpecialColoringProvider","Cafe2SpreadsheetProvider","prototype","InitSpreadsheet"},48,3559229741213067635ull,13}};
}
if(bank=="grid.modern"){contracts={{"_SetSpreadsheetDataColumns",{"_global","widget","Object","provider","SpreadsheetDataProvider","prototype","Init","mLegacyDataProvider"},64,9168494034957904426ull,26}};
}
if(bank=="recruiting.0"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","TopSchoolListItem","cafe","enum","InvalidationType"},142,8449965055051898562ull,110}};
}
if(bank=="recruiting.1"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","RecruitPlayerInfo","INVALIDATE_BACKGROUND","cafe","enum"},317,2246477310399147363ull,237},{"Draw",{"_global","art_control","Object","recruitinghub","RecruitPlayerInfo","INVALIDATE_BACKGROUND","cafe","enum"},317,2246477310399147363ull,237}};
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2536){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2552){q.byText=true;q.text=",";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2568){q.byText=true;q.text="split";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2584){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2600){q.byText=true;q.text="join";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2616){q.byText=true;q.text="isNaN";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2672){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2688){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2712){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2728){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2744){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2768){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2792){q.byText=true;q.text="M";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2840){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2856){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2880){q.byText=true;q.text="Math";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2896){q.byText=true;q.text="floor";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2912){q.byText=true;q.text="";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2936){q.byText=true;q.text=".";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2960){q.byText=true;q.text="K";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==2992){q.byText=true;q.text="cafe";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3008){q.byText=true;q.text="utility";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3024){q.byText=true;q.text="MathUtil";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3040){q.byText=true;q.text="FormatNumberString";}
for(auto& q:programs.at(1).sourcePointers)if(q.offset==3056){q.byText=true;q.text="$";}
}
if(bank=="recruiting.2"){contracts={{"_PopulateTopSchoolsList",{"_global","logic_cfm","Object","model","recruitinghub","board","Overview","fb_logic_com"},347,4887646166224038371ull,133}};
}
if(bank=="recruiting.3"){contracts={{"Draw",{"_global","art_control","Object","recruitinghub","PlayerListItem","cafe","enum","InvalidationType"},322,16900888385295740251ull,233},{"_LookFocused",{"_global","art_control","Object","recruitinghub","PlayerListItem","cafe","enum","InvalidationType"},322,16900888385295740251ull,290},{"_LookUnfocused",{"_global","art_control","Object","recruitinghub","PlayerListItem","cafe","enum","InvalidationType"},322,16900888385295740251ull,297}};
}
if(bank=="recruiting.6"){contracts={{"SetData",{"_global","madden","Object","component","TargetCell","SpreadsheetCell","prototype","Init"},84,6781013203620690527ull,25}};
}
if(bank=="recruiting.7"){contracts={{"Draw",{"_global","art_control","Object","module","spreadsheet","TopSchoolsInfoSlot","cafe","enum"},261,9533686984918159506ull,213}};
}
if(contracts.size()!=programs.size())throw std::runtime_error("Missing universal UI contracts for "+bank);
using Validator=std::function<bool(uintptr_t,const Header&,const notesui::GuardedRead&)>;std::vector<Validator> validators;
for(const auto& contract:contracts)validators.emplace_back(dependency?Validator{}:Validator([contract](uintptr_t definition,const Header& header,const notesui::GuardedRead& read){return declarationMatches(definition,contract,read)&&prefixMatches(contract,header,read,reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)))&&poolMatches(contract,header,read,reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)));}));
runtime.configureDiscovery(std::move(programs),std::move(validators));return contracts;
}
}
