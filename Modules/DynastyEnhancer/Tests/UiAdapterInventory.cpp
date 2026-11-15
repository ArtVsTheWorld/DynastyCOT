// Offline catalogue only: no startup, hooks, helper loading or game access.
#include "../Source/RecruitingFog.cpp"
#include <iostream>
int main(int argc,char**){using namespace recruitingfog;using J=nlohmann::json;J result=J::array();
    if(argc>1)runtime::configureUniversalUi();
    auto bank=[&](std::string name,const notesui::Runtime& runtime){J rows=J::array();for(const auto& p:runtime.discoveryPrograms()){
        J pointers=J::array(),nested=J::array();for(const auto& q:p.sourcePointers)pointers.push_back({{"offset",q.offset},{"value",q.value},{"text",q.text},{"byText",q.byText}});
        for(const auto& q:p.nestedDefinitions)nested.push_back({{"offset",q.offset},{"name",q.name},{"params",q.params},{"emptyPool",q.emptyPool}});
        rows.push_back({{"offset",p.offset},{"argc",p.argc},{"registers",p.registers},{"flags",p.flags},{"size",p.size},{"hash",p.hash},{"newRegisters",p.newRegisters},{"bytes",p.code.size()},{"pointers",pointers},{"nested",nested}});
    }result.push_back({{"bank",name},{"methods",rows}});};
    for(size_t i=0;i<runtime::cardUiBanks.size();++i)bank("card."+std::to_string(i),*runtime::cardUiBanks[i]);
    bank("grid.legacy",runtime::playerUi);bank("grid.modern",runtime::modernPlayerUi);bank("notes",runtime::notesUi);
    for(size_t i=0;i<features::banks.size();++i)bank("recruiting."+std::to_string(i),*features::banks[i]);
    std::cout<<result.dump(2)<<'\n';
}
