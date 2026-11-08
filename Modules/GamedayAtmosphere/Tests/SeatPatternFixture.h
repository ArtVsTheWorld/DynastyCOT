#pragma once
#include "../Source/SeatPattern.h"
namespace attendance::seats::fixture {
struct Layout {std::vector<Position> positions;std::vector<unsigned short> baseline;unsigned short parts=3;};
// Synthetic model positions for policy tests, not a capture of a real venue.
inline Layout bowl(unsigned rows=32,unsigned columns=288,bool sparse=false){
    Layout out;constexpr double pi=3.14159265358979323846;
    for(unsigned row=0;row<rows;++row)for(unsigned seat=0;seat<columns;++seat){
        double theta=2*pi*(seat+.5)/columns,c=std::cos(theta),s=std::sin(theta);
        double edge=1/(std::max)(std::abs(c)/29,std::abs(s)/60),radius=edge+row*.9;
        out.positions.push_back({static_cast<float>(c*radius),static_cast<float>(1+row*.65),static_cast<float>(s*radius),0});
        out.baseline.push_back(sparse&&(row*columns+seat)%17==0?3:0);
    }
    return out;
}
}
