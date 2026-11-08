#pragma once
#include <iomanip>
#include <ostream>
#include <string>

namespace attendance {
// One value per line, aligned after the colon. Keep labels intact so existing
// diagnostic readers can still find them while people can scan by section.
class StatusReport {
    std::ostream& out;
public:
    explicit StatusReport(std::ostream& stream,const char* version):out(stream){
        out<<std::fixed<<std::setprecision(1)<<"Gameday Atmosphere "<<version<<"\n";
    }
    void section(const char* title){out<<"\n=== "<<title<<" ===\n";}
    std::ostream& field(const char* label){
        out<<std::left<<std::setw(36)<<(std::string(label)+":")<<' ';
        return out;
    }
};
}
