#pragma once
#include <windows.h>
#include <filesystem>
#include <atomic>
#include <string>
#include "SettingsMenu.h"
#include "SettingsAptRouting.h"
#include "ConfigValue.h"

namespace attendance::settingsmenu::native {
using A=uintptr_t;
inline constexpr A ListSite=0x7f34030,ProviderGlobal=0xfb46b98,ProviderVtable=0xbcfbcf8,FileManagerGlobal=0xfa62218;
inline constexpr A PauseListSite=0x7f341f0,PauseProviderGlobal=0xfb46ba0,PauseProviderVtable=0xbcfc400;
inline constexpr std::array<unsigned char,16> ListPrefix={0x40,0x56,0x41,0x56,0x41,0x57,0x48,0x83,0xec,0x30,0x49,0x8b,0,0x48,0x8b,0xf1};
inline constexpr std::array<unsigned char,16> PauseListPrefix={0x41,0x55,0x41,0x56,0x48,0x83,0xec,0x48,0x48,0x83,0x79,0x08,0x00,0x4c,0x8b,0xea};
template<class T>inline T peek(A p){T v{};__try{std::memcpy(&v,reinterpret_cast<const void*>(p),sizeof(v));}__except(EXCEPTION_EXECUTE_HANDLER){return T{};}return v;}
inline bool copy(A p,void* out,size_t size){__try{std::memcpy(out,reinterpret_cast<const void*>(p),size);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
template<class R,class... Args>inline bool obtain(R& out,A fn,Args... args){__try{out=reinterpret_cast<R(__fastcall*)(Args...)>(fn)(args...);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
template<class... Args>inline bool invoke(A fn,Args... args){__try{reinterpret_cast<void(__fastcall*)(Args...)>(fn)(args...);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
inline int number(A reader){int n=-1;obtain(n,peek<A>(peek<A>(reader)+0x18),reader);return n;}
inline int field(A reader,const char* key){int n=-1;obtain(n,peek<A>(peek<A>(reader)+0x10),reader,key);return n;}
inline bool same(A p,const char* s){char b[96]{};size_t n=std::strlen(s);return n<sizeof(b)&&copy(p,b,n+1)&&!std::memcmp(b,s,n+1);}
struct ArrayWriter {
    A output;bool okay=true;
    void integer(int row,const char* key,int value){okay=invoke(peek<A>(peek<A>(output)+0x20),output,row,key,value)&&okay;}
    void text(int row,const char* key,const char* value){okay=invoke(peek<A>(peek<A>(output)+0x30),output,row,key,value)&&okay;}
    void boolean(int row,const char* key,bool value){okay=invoke(peek<A>(peek<A>(output)+0x18),output,row,key,value)&&okay;}
};
struct ObjectWriter {
    A output;bool okay=true;
    void integer(const char* key,int value){okay=invoke(peek<A>(peek<A>(output)+0x10),output,key,value)&&okay;}
};
struct Row {int type,pad;const char* title;const char* description;int key,pad2;uint64_t disabled,unselectable,hidden;};
struct CategoryRow {int key,pad;A rows;int count,pad2;};
static_assert(sizeof(Row)==56&&sizeof(CategoryRow)==24);
template<bool Pause> class BasicMenu {
    friend struct Fixture;
    A image_=0,original_=0,owner_=0,initNotify_=0,shutdownNotify_=0;std::array<A,42> stock_{};std::array<A,43> shadow_{};
    std::filesystem::path path_;using Note=void(*)(const char*);Note note_=nullptr;
    bool active_=false,layoutNoted_=false,relocatedNoted_=false;std::atomic<bool> scriptReady_{false},failed_{false};std::atomic<unsigned> displays_{0};
    std::array<double,Count> current_{};std::array<std::vector<double>,Count> choices_{};
    inline static BasicMenu* instance_=nullptr;
    static constexpr A ConcreteVtable=Pause?PauseProviderVtable:ProviderVtable;
    static void __fastcall list(A p,A out,A in){instance_->getList(p,out,in);}
    static void __fastcall init(A p,A in){instance_->initialize(p,in);}
    static void __fastcall shutdown(A p,A in){instance_->finish(p,in);}
    static void __fastcall toggleDetails(A p,A out,A category,A key){instance_->getToggle(p,out,category,key);}
    static void __fastcall sliderDetails(A p,A out,A category,A key){instance_->getSlider(p,out,category,key);}
    static void __fastcall saveToggle(A p,A out,A category,A key,A value,A reason){instance_->setToggle(p,out,category,key,value,reason);}
    static void __fastcall saveSlider(A p,A out,A category,A key,int value,A reason){instance_->setSlider(p,out,category,key,value,reason);}
    void fault(const char* what){if(!failed_&&note_)note_(what);failed_=true;}
    bool knownProvider(A p)const{
        A table=peek<A>(p);
        return p&&(table==image_+ConcreteVtable||(p==owner_&&table==reinterpret_cast<A>(shadow_.data()+1)));
    }
    // Called on the native menu thread. The cache lock pins the exact named APT
    // file; no heap scans, disk edits or changes to another movie are necessary.
    bool routeCategory(){
        scriptReady_=false;
        A manager=peek<A>(image_+FileManagerGlobal),cache=peek<A>(manager+0x20);
        if(!cache)return false;
        auto enter=peek<A>(image_+0x1178c01c),leave=peek<A>(image_+0x1178c024);
        if(!enter||!leave||!invoke(enter,cache+8))return false;
        bool ready=false;A node=peek<A>(cache);
        for(unsigned i=0;node&&i<2048;++i){
            A file=peek<A>(node),name=peek<A>(file+0x10);
            if(same(name+12,"fb_node_com/Scriptpod")&&peek<A>(file)==image_+0xbc403d8){
                A data=peek<A>(file+0x30),root=peek<A>(file+0x38);
                const auto replacement=categoryFallback();size_t offset=0;
                auto read=[&](size_t at,void* out,size_t size){return data&&data<=UINTPTR_MAX-routing::MaxOffset&&root==data+0x14a8&&at<=routing::MaxOffset&&size<=routing::MaxOffset-at&&copy(data+at,out,size);};
                if(routing::locate(read,replacement,offset,data)){
                    unsigned char actual[32]{};
                    if(copy(data+offset,actual,sizeof(actual))){
                        if(!std::memcmp(actual,replacement.data(),sizeof(actual)))ready=true;
                        else if(!std::memcmp(actual,routing::StockFallback.data(),sizeof(actual))){
                            DWORD prior=0;
                            if(VirtualProtect(reinterpret_cast<void*>(data+offset),sizeof(actual),PAGE_READWRITE,&prior)){
                                std::memcpy(reinterpret_cast<void*>(data+offset),replacement.data(),replacement.size());
                                DWORD unused=0;VirtualProtect(reinterpret_cast<void*>(data+offset),sizeof(actual),prior,&unused);ready=true;
                            }
                        }
                    }
                    if(ready&&offset!=0x2892c&&!relocatedNoted_){
                        relocatedNoted_=true;if(note_)note_("Settings compatibility: DGA routing added to the active replacement menu; existing broadcast and native controls preserved.");
                    }
                }
                break;
            }
            A next=peek<A>(node+8);if(next==node)break;node=next;
        }
        invoke(leave,cache+8);scriptReady_=ready;return ready;
    }
    bool attach(A provider){
        if(owner_==provider&&peek<A>(provider)==reinterpret_cast<A>(shadow_.data()+1))return true;
        if(peek<A>(provider)!=image_+ConcreteVtable||!copy(image_+ConcreteVtable,stock_.data(),sizeof(stock_)))return false;
        shadow_[0]=peek<A>(image_+ConcreteVtable-8);std::copy(stock_.begin(),stock_.end(),shadow_.begin()+1);
        auto table=shadow_.data()+1;table[1]=reinterpret_cast<A>(init);table[2]=reinterpret_cast<A>(shutdown);table[4]=reinterpret_cast<A>(list);
        table[5]=reinterpret_cast<A>(toggleDetails);table[6]=reinterpret_cast<A>(saveToggle);table[7]=reinterpret_cast<A>(sliderDetails);table[8]=reinterpret_cast<A>(saveSlider);
        if(InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(provider),table,reinterpret_cast<void*>(image_+ConcreteVtable))!=reinterpret_cast<void*>(image_+ConcreteVtable))return false;
        owner_=provider;active_=false;return true;
    }
    bool root(A p,A output){
        if constexpr(Pause)if(peek<int>(p+0x10)!=13)return false;
        std::array<CategoryRow,Pause?13:17> categories{};if(!copy(peek<A>(p+8),categories.data(),sizeof(categories)))return false;
        const CategoryRow* root=nullptr;for(const auto& c:categories)if(c.key==3)root=&c;
        if(!root||root->count!=(Pause?7:12))return false;
        std::array<Row,Pause?7:12> rows{};if(!copy(root->rows,rows.data(),sizeof(rows)))return false;
        constexpr std::array<int,12> frontendKeys={0,0,9,17,10,1,2,3,5,7,16,19};
        constexpr std::array<int,7> pauseKeys={0,9,10,1,5,7,16};
        const auto& keys=[&]()->const auto&{if constexpr(Pause)return pauseKeys;else return frontendKeys;}();
        for(size_t i=0;i<rows.size();++i)if(rows[i].type!=4||rows[i].key!=keys[i]||!rows[i].title)return false;
        if(!routeCategory()||!attach(p)){
            if(!layoutNoted_&&note_)note_("Settings tab not added: the active frontend routing or provider did not match. Native tabs and INI editing remain available.");
            layoutNoted_=true;return false;
        }
        uint64_t mask=0;if(!obtain(mask,image_+(Pause?0x7f2d890:0x7f2d3a0),p))return false;
        ArrayWriter out{output};int i=0;
        for(const auto& row:rows){
            if(row.hidden&mask)continue;
            out.integer(i,"type",row.type);out.text(i,"display",row.title);if(row.description)out.text(i,"description",row.description);out.integer(i,"key",row.key);
            bool rowEnabled=(row.disabled&mask)==0,selectable=(row.unselectable&mask)==0;
            out.boolean(i,"isEnabled",rowEnabled);out.boolean(i,"isInteractive",rowEnabled);out.boolean(i,"isSelectable",selectable);out.boolean(i,"isFocusable",selectable);++i;
        }
        out.integer(i,"type",4);out.text(i,"display","Gameday Atmosphere");out.text(i,"description","Gameday Atmosphere settings");out.integer(i,"key",Category);
        out.boolean(i,"isEnabled",true);out.boolean(i,"isInteractive",true);out.boolean(i,"isSelectable",true);out.boolean(i,"isFocusable",true);
        if(out.okay){++displays_;if(displays_==1&&note_)note_(Pause?"Gameday Atmosphere tab appended at the right of pause-menu settings.":"Gameday Atmosphere tab appended after the native settings tabs.");}
        else fault("Native settings field serialization failed; additional DGA menu writes disabled for this session.");
        return true;
    }
    double readOption(size_t i){
        const auto& o=Options[i];wchar_t raw[128]{};GetPrivateProfileStringW(o.section,o.key,L"",raw,128,path_.c_str());
        if(o.kind==Kind::Profile){for(int n=0;n<4;++n)if(!_wcsicmp(raw,ProfileValues[n]))return n;return 0;}
        if(o.kind!=Kind::Choice)return config::integer(raw,static_cast<int>(o.factory),static_cast<int>(o.low),static_cast<int>(o.high)).value;
        return config::real(raw,static_cast<float>(o.factory),static_cast<float>(o.low),static_cast<float>(o.high)).value;
    }
    void save(int key,double value){
        if(!active_||failed_||!owned(key)||!std::isfinite(value))return;
        size_t i=static_cast<size_t>(key-FirstKey);const auto& o=Options[i];
        if(value<o.low||value>o.high||std::abs(value-current_[i])<.00001)return;
        if(GetPrivateProfileIntW(L"General",L"ConfigVersion",2,path_.c_str())>2)return;
        wchar_t text[64]{};std::swprintf(text,64,L"%.9g",value);const wchar_t* saved=o.kind==Kind::Profile?ProfileValues[static_cast<int>(value)]:text;
        if(!WritePrivateProfileStringW(o.section,o.key,saved,path_.c_str())){fault("Could not save a DGA setting to its INI; menu writes disabled for this session.");return;}
        current_[i]=value;
    }
public:
    bool bindings()const{
        constexpr A entries[]={0x7f27c90,0x7f31cd0,0x7f32010,0x7f33cc0,0x7f34030,0x7f34fe0,0x7f305d0,0x7f34a40,0x7f2f830,0x7f339e0,0x7f2f7b0,0x7f440a0,0x7f34670,0x7f34650,0x7f345f0,0x7f34610,0x7f34630,0x7f2d360,0x7f40dc0,0x7f47b80,0x7f471c0,0x7f471b0,0x7f471d0,0x7f47b90,0x7f47710,0x7f471e0,0x7f32cf0,0x7f43680,0x7f42c80,0x7f27f40,0x78cd63a,0x78cd63a,0x7f27ee0,0x78cd63a,0x78cd63a,0x78cd63a,0x78cd63a,0x78cd63a,0x78cd63a,0x78cd63a,0x78cd63a,0x78cd63a};
        constexpr A pauseEntries[]={0x7f27d90,0x7f31f50,0x7f321b0,0x7f33cd0,0x7f341f0,0x7f35240,0x7f30e10,0x7f34c10,0x7f2fda0,0x7f33a30,0x7f2f820,0x7f440b0,0x7f34680,0x7f34660,0x7f34600,0x7f34620,0x7f34640,0x7f42fe0,0x7f27f10};
        for(size_t i=0;i<stock_.size();++i){A expected=Pause?(i<std::size(pauseEntries)?pauseEntries[i]:0x78cd63a):entries[i];if(peek<A>(image_+ConcreteVtable+8*i)!=image_+expected)return false;}
        return true;
    }
    void configure(A moduleBase,const std::filesystem::path& path,Note note){image_=moduleBase;path_=path;note_=note;instance_=this;initNotify_=moduleBase+0x7a6b950;shutdownNotify_=moduleBase+0x7a6bb90;}
    void original(A p){original_=p;}
    static A callback(){return reinterpret_cast<A>(list);}
    bool ready()const{return scriptReady_&&!failed_;}
    unsigned displays()const{return displays_;}
    void getList(A p,A out,A in){
        int category=number(in);
        if(!failed_&&knownProvider(p)){
            if(category==Category&&scriptReady_){ArrayWriter writer{out};settingsmenu::list(writer);if(!writer.okay)fault("Could not render DGA settings controls; menu writes disabled for this session.");return;}
            if(category==3&&GetPrivateProfileIntW(L"General",L"ConfigVersion",2,path_.c_str())<=2&&root(p,out))return;
        }
        invoke(original_,p,out,in);
    }
    void initialize(A p,A in){
        if(field(in,"key")!=Category){active_=false;invoke(stock_[1],p,in);return;}
        owner_=p;active_=true;
        for(size_t i=0;i<Count;++i){current_[i]=readOption(i);choices_[i]=Choices(Options[i],current_[i]).values;}
        if constexpr(Pause)*reinterpret_cast<unsigned char*>(p+0x34)=1;
        else *reinterpret_cast<unsigned char*>(p+0x62)=1;
        invoke(initNotify_,image_+0xdab8be0);
    }
    void finish(A p,A in){
        if(!active_){invoke(stock_[2],p,in);return;}
        active_=false;if constexpr(!Pause)*reinterpret_cast<unsigned char*>(p+0x62)=0;*reinterpret_cast<unsigned char*>(p+0x34)=0;
        invoke(shutdownNotify_,image_+0xdab8be0,Category,true,"");
    }
    void getToggle(A p,A out,A category,A key){
        int id=number(key);
        if(number(category)!=Category){invoke(stock_[5],p,out,category,key);return;}
        if(!active_||p!=owner_||!owned(id)||find(id)->kind==Kind::Slider)return;
        size_t i=static_cast<size_t>(id-FirstKey);Choices choices(Options[i],current_[i]);choices.values=choices_[i];ArrayWriter writer{out};settingsmenu::toggle(writer,Options[i],choices,current_[i]);
    }
    void getSlider(A p,A out,A category,A key){
        int id=number(key);
        if(number(category)!=Category){invoke(stock_[7],p,out,category,key);return;}
        if(!active_||p!=owner_||!owned(id)||find(id)->kind!=Kind::Slider)return;
        ObjectWriter writer{out};settingsmenu::slider(writer,*find(id),current_[static_cast<size_t>(id-FirstKey)]);
    }
    void setToggle(A p,A out,A category,A key,A value,A reason){
        if(number(category)!=Category){invoke(stock_[6],p,out,category,key,value,reason);return;}
        if(!active_||p!=owner_)return;
        int id=number(key),index=number(value);if(!owned(id)||find(id)->kind==Kind::Slider||index<0)return;
        const auto& values=choices_[static_cast<size_t>(id-FirstKey)];if(static_cast<size_t>(index)<values.size())save(id,values[index]);
    }
    void setSlider(A p,A out,A category,A key,int value,A reason){
        if(number(category)!=Category){invoke(stock_[8],p,out,category,key,value,reason);return;}
        if(!active_||p!=owner_)return;
        int id=number(key);if(owned(id)&&find(id)->kind==Kind::Slider)save(id,value);
    }
};
using Menu=BasicMenu<false>;
using PauseMenu=BasicMenu<true>;
}
