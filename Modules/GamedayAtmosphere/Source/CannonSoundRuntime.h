#pragma once
#include "CannonImpact.h"
#include "SwellAudioRuntime.h"

namespace attendance::cannon {
using sustained::Address;
// Called only inside the native sound entity's owning event callback.
// Inspect the final patch/selector after the original start has run. Nothing
// from this sound entity is retained or dereferenced on a later frame.
template<class Read,class Integer,class Playing>
int started(Address image,Address entity,Address event,Read read,Integer integer,Playing playing){
    auto number=[&](Address p,auto& value){return p>=0x10000&&read(p,&value,sizeof(value));};
    Address vt=0,data=0,patch=0,name=0,instance=0,begin=0,end=0;unsigned hash=0;unsigned char soundEnabled=0;float amplitude=0;
    if(!number(entity,vt)||vt!=image+bindings::LegacySoundVtable||!number(event+8,hash)||hash!=PlayEvent||
       !number(entity+0xe8,soundEnabled)||soundEnabled!=1||!number(entity+0x38,data)||!number(data+0x60,patch)||
       !number(data+0x68,amplitude)||!std::isfinite(amplitude)||amplitude<=0)return -1;
    patch&=~Address{4};
    char text[sizeof("Sound/CFB/CoreAudio/SoundPatches/Stadium/SoundEffects/FX_STADIUM_SPECIFIC_FX_SP")]{};
    if(!number(patch+0x18,name)||!read(name,text,sizeof(text))||text[sizeof(text)-1]!=0)return -1;
    for(size_t i=0;i<sizeof(text)-1;++i){auto lower=[](char c){return c>='A'&&c<='Z'?c+('a'-'A'):c;};if(lower(text[i])!=lower(Patch[i]))return -1;}
    if(
       !number(entity+0x98,instance)||!instance||!playing(instance)||
       !number(entity+0x58,begin)||!number(entity+0x60,end)||end<begin||end-begin>64*0x28||(end-begin)%0x28)return -1;
    for(Address row=begin;row<end;row+=0x28){
        unsigned parameter=0;int type=-1,value=-1;
        if(number(row+0x20,parameter)&&parameter==EffectParameter&&number(row+0x24,type)&&type==3&&integer(row,value)&&variant(value))return value;
    }
    return -1;
}
}
