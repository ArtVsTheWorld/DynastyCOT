#pragma once
#include <array>
#include <cstdint>
#include <cstring>

namespace attendance::replaypresentation {
using Address=uintptr_t;
using Read=bool(*)(Address,void*,size_t);
struct Types {Address logic=0,comparison=0,constant=0,function=0,actorFunction=0;};
enum class MajorRoute {None,Touchdown,Takeaway,QuickSkip,QuickTouchdown};
enum class DefenseRoute {None,QuickOffense,TackleForLoss};
struct Selection {Address fullComparison=0,insetGate=0,denyRoot=0;DefenseRoute defense=DefenseRoute::None;};

// Recognize authored selectors rather than retaining pointers to one match's
// expression objects. All reads are bounded and all shapes must match exactly.
class View {
    Read read_;Types types_;
    template<class T>static T field(const unsigned char* bytes,size_t offset){T value{};std::memcpy(&value,bytes+offset,sizeof(value));return value;}
public:
    View(Read read,Types types):read_(read),types_(types){}
    bool boolean(Address value,bool expected)const{
        std::array<unsigned char,0x10> bytes{};
        return value&&read_(value,bytes.data(),bytes.size())&&field<int>(bytes.data(),8)==4&&bytes[0]==static_cast<unsigned char>(expected);
    }
    bool falseBoolean(Address value)const{return boolean(value,false);}
    bool trueBoolean(Address value)const{return boolean(value,true);}
    bool function(Address node,const char* id)const{
        std::array<unsigned char,0x50> bytes{};
        if(!node||!read_(node,bytes.data(),bytes.size()))return false;
        Address vt=field<Address>(bytes.data(),0);
        if(vt!=types_.function&&vt!=types_.actorFunction)return false;
        size_t count=std::strlen(id);return count<8&&std::memcmp(bytes.data()+0x48,id,count+1)==0;
    }
    bool constant(Address node,int type,int value)const{
        std::array<unsigned char,0x18> bytes{};
        if(!node||!read_(node,bytes.data(),bytes.size())||field<Address>(bytes.data(),0)!=types_.constant||field<int>(bytes.data(),0x10)!=type)return false;
        return type==4?bytes[8]==static_cast<unsigned char>(value):field<int>(bytes.data(),8)==value;
    }
    bool compare(Address node,int op,const char* id,int type,int value)const{
        std::array<unsigned char,0x30> bytes{};
        return node&&read_(node,bytes.data(),bytes.size())&&field<Address>(bytes.data(),0)==types_.comparison&&
            field<int>(bytes.data(),0x28)==op&&function(field<Address>(bytes.data(),8),id)&&constant(field<Address>(bytes.data(),0x18),type,value);
    }
    template<size_t N>bool children(Address node,int op,std::array<Address,N>& out)const{
        static_assert(N<=12,"Bounded native expression inspection");
        std::array<unsigned char,0x50> bytes{};std::array<unsigned char,N*16> entries{};
        if(!node||!read_(node,bytes.data(),bytes.size())||field<Address>(bytes.data(),0)!=types_.logic||field<int>(bytes.data(),0x48)!=op)return false;
        Address begin=field<Address>(bytes.data(),8),end=field<Address>(bytes.data(),0x10);
        if(!begin||begin>UINTPTR_MAX-N*16||end!=begin+N*16||!read_(begin,entries.data(),entries.size()))return false;
        for(size_t i=0;i<N;++i){out[i]=field<Address>(entries.data(),i*16);if(!out[i])return false;}
        return true;
    }
    bool milestone(Address node,int down,int yards)const{
        std::array<Address,2> c{};return children(node,0,c)&&compare(c[0],0,"128",0,down)&&compare(c[1],5,"22",0,yards);
    }
    bool routineGate(Address node)const{
        std::array<Address,5> c{};
        return children(node,1,c)&&function(c[0],"257")&&function(c[1],"36")&&function(c[2],"38")&&milestone(c[3],2,5)&&milestone(c[4],3,7);
    }
    bool pair(Address node,const char* first,const char* second)const{
        std::array<Address,2> c{};return children(node,0,c)&&function(c[0],first)&&function(c[1],second);
    }
    bool lossGate(Address node)const{
        std::array<Address,2> c{};return children(node,0,c)&&function(c[0],"30")&&compare(c[1],2,"22",0,0);
    }
    bool quickOffense(Address node)const{
        std::array<Address,2> root{},sack{};std::array<Address,4> plays{};
        return children(node,0,root)&&function(root[0],"570")&&children(root[1],1,plays)&&
            pair(plays[0],"19","20")&&lossGate(plays[1])&&pair(plays[2],"60","20")&&
            children(plays[3],1,sack)&&function(sack[0],"17")&&pair(sack[1],"31","30");
    }
    MajorRoute major(Address node)const{
        std::array<Address,1> one{};
        if(children(node,0,one)){
            if(function(one[0],"27"))return MajorRoute::Touchdown;
            if(compare(one[0],0,"556",4,1))return MajorRoute::QuickSkip;
        }
        std::array<Address,2> two{};std::array<Address,1> guard{};
        if(children(node,0,two)&&function(two[0],"16")&&children(two[1],0,guard)&&compare(guard[0],0,"1042",4,0))return MajorRoute::Takeaway;
        std::array<Address,3> three{};std::array<Address,9> reasons{};
        if(children(node,0,three)&&children(three[0],1,reasons)&&compare(reasons[0],4,"178",0,19)&&
           compare(three[1],0,"157",4,0)&&compare(three[2],0,"556",4,1)){
            for(size_t i=1;i<reasons.size();++i)if(!compare(reasons[i],5,"540",0,20))return MajorRoute::None;
            return MajorRoute::QuickTouchdown;
        }
        return MajorRoute::None;
    }
    bool challengeExcluded(Address node)const{
        std::array<Address,2> two{};std::array<Address,1> guard{};std::array<unsigned char,0x30> comparison{};std::array<unsigned char,0x18> fn{};
        return children(node,0,two)&&children(two[1],0,guard)&&read_(guard[0],comparison.data(),comparison.size())&&
            read_(field<Address>(comparison.data(),8),fn.data(),fn.size())&&field<int>(fn.data(),0x10)==4&&fn[8]==0;
    }
    Selection identify(Address node)const{
        std::array<Address,2> two{};
        if(children(node,0,two)){
            if(function(two[0],"1")&&compare(two[1],5,"22",0,40))return {two[1],0};
            if((function(two[1],"0")||function(two[1],"1"))&&routineGate(two[0]))return {0,two[0]};
        }
        std::array<Address,3> three{};
        if(children(node,0,three)&&function(three[0],"0")&&compare(three[1],0,"14",4,0)&&compare(three[2],5,"22",0,35))return {three[2],0};
        if(children(node,0,three)&&function(three[0],"30")&&compare(three[1],2,"22",0,0)&&compare(three[2],0,"31",4,0))return {0,0,node,DefenseRoute::TackleForLoss};
        if(quickOffense(node))return {0,0,node,DefenseRoute::QuickOffense};
        return {};
    }
    bool fullEligible(Address comparison,int minimumYards)const{
        std::array<unsigned char,0x30> bytes{};
        if(minimumYards<10||minimumYards>60||!read_(comparison,bytes.data(),bytes.size())||field<Address>(bytes.data(),0)!=types_.comparison||field<int>(bytes.data(),0x28)!=5)return false;
        Address left=field<Address>(bytes.data(),8),right=field<Address>(bytes.data(),0x18);
        if(!function(left,"22")||(!constant(right,0,40)&&!constant(right,0,35)))return false;
        std::array<unsigned char,0x18> value{};
        if(!read_(left,value.data(),value.size())||field<int>(value.data(),0x10)!=0)return false;
        int yards=field<int>(value.data(),8);return yards>=minimumYards&&yards<=110;
    }
};

struct ThreadState {unsigned depth=0;Selection selection{};};
class Scope {
    ThreadState& state_;ThreadState previous_;
public:
    Scope(ThreadState& state,const View& view,Address node,bool allowed,bool full,bool inset,bool defensive=false):state_(state),previous_(state){
        ++state_.depth;
        if(previous_.depth==0){
            state_.selection=allowed?view.identify(node):Selection{};
            if(!full)state_.selection.fullComparison=0;if(!inset)state_.selection.insetGate=0;
            if(!defensive||(!inset&&state_.selection.defense==DefenseRoute::TackleForLoss)){state_.selection.denyRoot=0;state_.selection.defense=DefenseRoute::None;}
        }
    }
    ~Scope(){state_=previous_;}
    Scope(const Scope&)=delete;Scope& operator=(const Scope&)=delete;
};
}
