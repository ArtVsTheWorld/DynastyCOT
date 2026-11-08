#pragma once
#include <array>
namespace attendance::audience {
// Frozen evidence from D1.ticker 2025 final and five-year attendance.
// Lower historical audience / reported 2025 capacity. This is an observed
// turnout proxy, never the player-selected venue or a list of favored schools.
// https://www.d1ticker.com/2025-fbs-attendance-trends/
// Source snapshot SHA256 0d51f2d5550db86d6b801d8f1186eb2178567abcc41d0399460fce7dcf5dee52.
struct TurnoutEvidence {int id;float lowerHistoricalFill;};
inline constexpr std::array<TurnoutEvidence,134> TurnoutHistory{{
 {1100,0.672854517f}, // Air Force
 {1101,0.257600000f}, // Akron
 {1102,0.997172177f}, // Alabama
 {1103,1.047767064f}, // Appalachian State
 {1104,0.822800000f}, // Arizona
 {1105,0.883554199f}, // Arizona State
 {1106,0.860997196f}, // Arkansas
 {1107,0.537805865f}, // Arkansas State
 {1108,0.946300000f}, // Army
 {1109,0.984064605f}, // Auburn
 {1110,0.398600000f}, // Ball State
 {1111,0.873900000f}, // Baylor
 {1112,0.903900000f}, // Boise State
 {1113,0.812992013f}, // Boston College
 {1114,0.411278379f}, // Bowling Green
 {1115,0.552000000f}, // Buffalo
 {1116,0.984797429f}, // BYU
 {1117,0.560100000f}, // California
 {1118,0.443841371f}, // Central Michigan
 {1119,0.824385321f}, // Charlotte
 {1120,0.946500000f}, // Cincinnati
 {1121,0.971100000f}, // Clemson
 {1122,0.791567115f}, // Coastal Carolina
 {1123,0.974832947f}, // Colorado
 {1124,0.667901078f}, // Colorado State
 {1125,0.552749527f}, // UConn
 {1234,0.800900722f}, // Delaware
 {1126,0.668385858f}, // Duke
 {1128,0.711223355f}, // East Carolina
 {1129,0.565100000f}, // Florida International
 {1130,0.986224202f}, // Florida
 {1131,0.588900000f}, // Florida Atlantic
 {1132,0.927947926f}, // Florida State
 {1133,0.906421956f}, // Fresno State
 {1134,0.997688992f}, // Georgia
 {1135,0.736767313f}, // Georgia Southern
 {1136,0.637000000f}, // Georgia State
 {1137,0.736766811f}, // Georgia Tech
 {1138,0.778993230f}, // Hawai'i
 {1139,0.702451932f}, // Houston
 {1140,0.723385686f}, // Illinois
 {1141,0.866644670f}, // Indiana
 {1142,0.982194946f}, // Iowa
 {1143,0.962934034f}, // Iowa State
 {1144,0.800464463f}, // Jacksonville State
 {1145,0.943267794f}, // James Madison
 {1146,0.887533525f}, // Kansas
 {1147,0.999418606f}, // Kansas State
 {1233,0.683408471f}, // Kennesaw State
 {1149,0.947200000f}, // Kentucky
 {1150,0.734900000f}, // Liberty
 {1151,0.898550000f}, // Louisiana
 {1152,0.479000000f}, // Louisiana Tech
 {1153,0.781885914f}, // Louisville
 {1154,0.970476030f}, // LSU
 {1155,0.741538062f}, // Marshall
 {1156,0.797490210f}, // Maryland
 {1157,0.601900000f}, // Memphis
 {1158,0.806387367f}, // Miami
 {1159,0.387500000f}, // Miami University
 {1160,1.024105751f}, // Michigan
 {1161,0.915021677f}, // Michigan State
 {1162,0.448700000f}, // Middle Tennessee
 {1163,0.915600000f}, // Minnesota
 {1164,0.871387373f}, // Mississippi State
 {1165,0.887884182f}, // Missouri
 {1235,0.521405993f}, // Missouri State
 {1166,0.940000000f}, // Navy
 {1167,0.986489573f}, // NC State
 {1168,1.012800000f}, // Nebraska
 {1169,0.644963916f}, // Nevada
 {1170,0.402948638f}, // New Mexico
 {1171,0.329400000f}, // New Mexico State
 {1172,0.966970297f}, // North Carolina
 {1173,0.650049724f}, // North Texas
 {1174,0.330000000f}, // Northern Illinois
 {1175,0.960500000f}, // Northwestern
 {1176,0.980495220f}, // Notre Dame
 {1177,0.737757024f}, // Ohio
 {1178,0.997887227f}, // Ohio State
 {1179,1.028721807f}, // Oklahoma
 {1180,0.853900000f}, // Oklahoma State
 {1181,0.843473497f}, // Old Dominion
 {1182,0.931593391f}, // Ole Miss
 {1183,1.011193025f}, // Oregon
 {1184,0.848000000f}, // Oregon State
 {1185,1.004900000f}, // Penn State
 {1186,0.715366612f}, // Pittsburgh
 {1187,0.920900000f}, // Purdue
 {1188,0.419266260f}, // Rice
 {1189,0.850462568f}, // Rutgers
 {1190,0.340900000f}, // Sam Houston
 {1191,0.686125011f}, // San Diego State
 {1192,0.769000000f}, // San Jose State
 {1193,0.794021778f}, // SMU
 {1194,0.663336618f}, // South Alabama
 {1195,0.981576047f}, // South Carolina
 {1196,0.660471604f}, // Southern Mississippi
 {1197,0.558700000f}, // Stanford
 {1198,0.881732094f}, // Syracuse
 {1199,0.921700000f}, // TCU
 {1200,0.259900000f}, // Temple
 {1201,0.939233675f}, // Tennessee
 {1202,0.983777975f}, // Texas
 {1203,0.980719563f}, // Texas A&M
 {1204,0.654000000f}, // Texas State
 {1205,0.915880063f}, // Texas Tech
 {1206,0.764867668f}, // Toledo
 {1207,0.789400000f}, // Troy
 {1208,0.694728864f}, // Tulane
 {1209,0.545600000f}, // Tulsa
 {1210,0.418300000f}, // UAB
 {1211,0.962884379f}, // UCF
 {1212,0.415600000f}, // UCLA
 {1213,0.533569410f}, // UL Monroe
 {1214,0.517000000f}, // Massachusetts
 {1215,0.369842477f}, // UNLV
 {1216,0.819852824f}, // USC
 {1217,0.498959089f}, // USF
 {1218,0.994464227f}, // Utah
 {1219,0.738529674f}, // Utah State
 {1220,0.307100000f}, // UTEP
 {1221,0.380600000f}, // UTSA
 {1222,0.656970166f}, // Vanderbilt
 {1223,0.693572259f}, // Virginia
 {1224,0.913400000f}, // Virginia Tech
 {1225,0.927764944f}, // Wake Forest
 {1226,0.944319768f}, // Washington
 {1227,0.779175180f}, // Washington State
 {1228,0.862518350f}, // West Virginia
 {1229,0.668000000f}, // Western Kentucky
 {1230,0.460513895f}, // Western Michigan
 {1231,0.925700000f}, // Wisconsin
 {1232,0.734270337f}, // Wyoming
}};
}
