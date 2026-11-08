#pragma once
namespace creationtests {
inline void run(){
    constexpr uintptr_t image=0x140000000,provider=0x200000000,response=0x210000000,data=0x220000000,service=0x230000000;
    std::map<uintptr_t,std::vector<unsigned char>> memory;
    auto put=[&](uintptr_t at,auto value){std::vector<unsigned char> bytes(sizeof(value));memcpy(bytes.data(),&value,sizeof(value));memory[at]=std::move(bytes);};
    auto seed=[&](){memory.clear();put(provider,image+0xbab9a08);put(provider+0x418,response);put(provider+0x420,uint8_t{1});
        put(image+0xd9eb808,int{-1});put(image+0xfa519b8,service);put(service,image+0xbaa8998);put(service+0x7b1,uint8_t{0});
        put(response,image+0xbd7d2b8);put(image+0xbd7d2b8+0xa10,image+0x8459ff0);
        memory[image+0x8459ff0]={0x32,0xc0,0xc3};put(response+0xb1,uint8_t{1});put(response+0xb4,int{1});put(response+0xc4,int{0});put(response+0x140,data);put(image+0xf55c998,uint32_t{1322026403});};
    auto read=[&](uintptr_t at,void* output,size_t size){const auto found=memory.find(at);if(found==memory.end()||found->second.size()!=size)return false;memcpy(output,found->second.data(),size);return true;};
    seed();auto scope=enhancements::creationScope(image,provider,read);
    require(scope&&scope.league==1322026403&&scope.response==response&&scope.data==data,"Validated creation page was not admitted before the career service activates");
    for(const auto& mutation:std::vector<std::pair<uintptr_t,std::vector<unsigned char>>>{{provider,{}},{provider+0x418,{}},{response+0x140,{}},{image+0xf55c998,{}},{image+0x8459ff0,{0xb0,1,0xc3}}}){
        seed();memory[mutation.first]=mutation.second;require(!enhancements::creationScope(image,provider,read),"Unreadable/online creation scope was admitted");}
    for(const uint32_t id:{0u,UINT32_MAX}){seed();put(image+0xf55c998,id);require(!enhancements::creationScope(image,provider,read),"Invalid creation LeagueID admitted");}
    for(const int mode:{-2,0,14,15,17}){seed();put(image+0xd9eb808,mode);require(!enhancements::creationScope(image,provider,read),"Other game mode admitted as creation");}
    seed();put(service+0x7b1,uint8_t{1});require(!enhancements::creationScope(image,provider,read),"Loaded career was mistaken for a creation");
    seed();put(provider+0x420,uint8_t{0});require(!enhancements::creationScope(image,provider,read),"Ordinary settings page was mistaken for initial creation");
    seed();put(response+0xb1,uint8_t{0});require(!enhancements::creationScope(image,provider,read),"Inactive native response admitted");
    seed();put(response+0xc4,int{1});require(!enhancements::creationScope(image,provider,read),"Invalid native response status admitted");
    seed();unsigned identities=0;auto changing=[&](uintptr_t at,void* output,size_t size){if(at==image+0xf55c998&&++identities==2)put(at,uint32_t{2819197555});return read(at,output,size);};
    require(!enhancements::creationScope(image,provider,changing),"Creation identity changed during validation but was admitted");
    std::cout<<"Passed: initial offline creation scope, native local-response fingerprint, allocated LeagueID, loaded/other-mode/online/inactive rejection and identity-change guard.\n";
}
}
