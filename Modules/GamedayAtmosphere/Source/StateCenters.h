#pragma once
#include <cmath>
#include <string_view>
namespace attendance::geography {
struct Point {double latitude=NAN,longitude=NAN;};
struct State {const char* name;Point point;};
// Census 2025 representative internal points, not exact city distances.
// Source: https://www2.census.gov/geo/docs/maps-data/data/gazetteer/2025_Gazetteer/2025_Gaz_state_national.zip
inline constexpr State States[]={
{"alabama",{32.739579,-86.843447}},
{"alaska",{63.347356,-152.839733}},
{"arizona",{34.203936,-111.606345}},
{"arkansas",{34.895526,-92.444626}},
{"california",{37.155177,-119.543418}},
{"colorado",{38.993767,-105.508712}},
{"connecticut",{41.579864,-72.746657}},
{"delaware",{38.998566,-75.441644}},
{"districtofcolumbia",{38.904243,-77.016524}},
{"florida",{28.398978,-82.5143}},
{"georgia",{32.629579,-83.423511}},
{"hawaii",{19.828167,-155.495042}},
{"idaho",{44.348422,-114.558854}},
{"illinois",{40.102875,-89.152611}},
{"indiana",{39.901314,-86.291913}},
{"iowa",{42.070023,-93.493349}},
{"kansas",{38.498546,-98.38343}},
{"kentucky",{37.533684,-85.29298}},
{"louisiana",{30.708319,-91.604621}},
{"maine",{45.409284,-68.666616}},
{"maryland",{38.946658,-76.674494}},
{"massachusetts",{42.15652,-71.489592}},
{"michigan",{44.844177,-85.660491}},
{"minnesota",{46.315957,-94.199604}},
{"mississippi",{32.686471,-89.656138}},
{"missouri",{38.350751,-92.456782}},
{"montana",{47.051177,-109.634817}},
{"nebraska",{41.543305,-99.811864}},
{"nevada",{39.331093,-116.615147}},
{"newhampshire",{43.672794,-71.584189}},
{"newjersey",{40.107274,-74.665201}},
{"newmexico",{34.434684,-106.131618}},
{"newyork",{42.913397,-75.596272}},
{"northcarolina",{35.53971,-79.130864}},
{"northdakota",{47.442174,-100.460826}},
{"ohio",{40.41493,-82.711997}},
{"oklahoma",{35.590081,-97.486779}},
{"oregon",{43.971712,-120.622958}},
{"pennsylvania",{40.904604,-77.827523}},
{"rhodeisland",{41.596485,-71.52649}},
{"southcarolina",{33.874178,-80.854264}},
{"southdakota",{44.446796,-100.238176}},
{"tennessee",{35.85846,-86.349634}},
{"texas",{31.434703,-99.281824}},
{"utah",{39.334993,-111.656333}},
{"vermont",{44.058954,-72.671017}},
{"virginia",{37.522251,-78.668194}},
{"washington",{47.407324,-120.5758}},
{"westvirginia",{38.647285,-80.618327}},
{"wisconsin",{44.630907,-89.709392}},
{"wyoming",{42.989659,-107.544392}},
{"puertorico",{18.217648,-66.410799}},
};
inline Point center(std::string_view value){
 char key[64]{};size_t count=0;for(char c:value){if(c>='A'&&c<='Z')c+=32;if(c>='a'&&c<='z'){if(count==63)return {};key[count++]=c;}}
 std::string_view name(key,count);if(name=="washingtondc"||name=="dc")name="districtofcolumbia";
 for(const auto& s:States)if(name==s.name)return s.point;return {};
}
}
