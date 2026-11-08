"""Compile Census state representative points, keyed by native enum label."""
from pathlib import Path
import json,re
ROOT=Path(__file__).resolve().parents[1]
data=json.loads((ROOT/'Data/Geography/state-points-2025.json').read_text())
out=['#pragma once','#include <cmath>','#include <string_view>','namespace attendance::geography {',
     'struct Point {double latitude=NAN,longitude=NAN;};',
     'struct State {const char* name;Point point;};',
     '// Census 2025 representative internal points, not exact city distances.',
     '// Source: '+data['source'],'inline constexpr State States[]={']
for row in data['states']:
    name=re.sub('[^a-z]','',row['NAME'].lower())
    out.append('{"'+name+'",{'+row['INTPTLAT']+','+row['INTPTLONG']+'}},')
out+=['};','inline Point center(std::string_view value){',
      ' char key[64]{};size_t count=0;for(char c:value){if(c>=\'A\'&&c<=\'Z\')c+=32;if(c>=\'a\'&&c<=\'z\'){if(count==63)return {};key[count++]=c;}}',
      ' std::string_view name(key,count);if(name=="washingtondc"||name=="dc")name="districtofcolumbia";',
      ' for(const auto& s:States)if(name==s.name)return s.point;return {};','}','}']
(ROOT/'Modules/GamedayAtmosphere/Source/StateCenters.h').write_text('\n'.join(out)+'\n')
