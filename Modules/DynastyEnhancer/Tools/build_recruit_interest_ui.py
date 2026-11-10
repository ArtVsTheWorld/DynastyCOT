"""Recruiting-only display suffixes; native bodies verified/copied at runtime."""
from pathlib import Path
import argparse,json,sys
from build_recruit_notes_ui import Assembler,fnv
from build_player_card_ui import path

def visible(a,widget,show,tag):
 path(a,1,widget);a.store(2);a.missing(2,tag);a.set_field(2,'_visible',show);a.label(tag)
def interest_row(a):
 # Only registers 0,2,3 are scratch; receiver is 1, native frame has 4.
 a.grade_scale();a.store(0)
 for mode in [0,2]:a.reg(0);a.number(mode);a.raw(0x66);a.branch('nativeRank'+str(mode))
 a.reg(0);a.number(3);a.raw(0x66);a.branch('blind')
 path(a,1,'mcPosition');a.store(2);a.missing(2,'done')
 path(a,1,'mBTFNativeRankX');a.raw(0x76,0x49);a.branch('savedRank',0xB8)
 a.reg(1);a.string('mBTFNativeRankX');a.reg(2);a.member('_x');a.raw(0x4F)
 a.reg(1);a.string('mBTFNativeRankWidth');a.reg(2);a.member('_width');a.raw(0x4F)
 a.label('savedRank');path(a,1,'mcTopSlider');a.store(3);a.missing(3,'done')
 a.reg(2);a.string('_x');a.reg(3);a.member('_x');a.raw(0x4F);a.set_field(2,'_visible',True)
 a.empty_object(3);a.set_field(3,'resizeWidth',310);a.reg(3);a.string('hAlign');a.global_object('art_styleguide','Font','HALIGN_LEFT');a.raw(0x4F);a.reg(3);a.number(1);a.reg(2);a.call('SetExtraInfo')
 path(a,1,'mData','ExtraInfo','SliderValue');a.store(3)
 a.reg(3);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('unknown',0xB8)
 for threshold,label in [(79,'FAVORITE'),(59,'CONTENDER'),(39,'IN THE MIX'),(19,'OUTSIDE LOOKING IN'),(0,'LONG SHOT')]:
  a.reg(3);a.number(threshold);a.raw(0x67);a.branch('tier'+str(threshold))
 a.label('unknown');a.string('INTEREST UNKNOWN');a.store(3);a.branch('setTier',0x99)
 for threshold,label in [(79,'FAVORITE'),(59,'CONTENDER'),(39,'IN THE MIX'),(19,'OUTSIDE LOOKING IN'),(0,'LONG SHOT')]:a.label('tier'+str(threshold));a.string(label);a.store(3);a.branch('setTier',0x99)
 a.label('setTier');a.reg(3);a.number(1);a.reg(2);a.call('SetLabel');visible(a,'mcTopSlider',False,'hideFeedbackBar');a.branch('done',0x99)
 a.label('blind');visible(a,'mcPosition',False,'hideBlindRank');visible(a,'mcTopSlider',False,'hideBlindBar');visible(a,'mcUpArrow',False,'hideUp');visible(a,'mcDownArrow',False,'hideDown');a.branch('done',0x99)
 for mode in [0,2]:
  a.label('nativeRank'+str(mode));path(a,1,'mcPosition');a.store(2);a.missing(2,'done');a.set_field(2,'_visible',True)
  path(a,1,'mBTFNativeRankX');a.raw(0x76,0x49);a.branch('restoreEnd'+str(mode))
  a.reg(2);a.string('_x');path(a,1,'mBTFNativeRankX');a.raw(0x4F)
  a.empty_object(3);a.reg(3);a.string('resizeWidth');path(a,1,'mBTFNativeRankWidth');a.raw(0x4F);a.reg(3);a.string('hAlign');a.global_object('art_styleguide','Font','HALIGN_CENTER');a.raw(0x4F);a.reg(3);a.number(1);a.reg(2);a.call('SetExtraInfo')
  a.label('restoreEnd'+str(mode));visible(a,'mcTopSlider',mode==0,'restoreBar'+str(mode));a.branch('done',0x99)
 a.label('done');a.ret()

def scout_row(a):
 # This row uses native currencies; interest mode is an independent immutable
 # renderer choice, not a new property that native SetData might discard.
 a.grade_scale();a.number(1);a.raw(0x66);a.branch('hideRank')
 a.grade_scale();a.number(3);a.raw(0x66);a.branch('hideRank')
 visible(a,'mcUserRanking',True,'rankRestored');a.branch('done',0x99)
 a.label('hideRank');visible(a,'mcUserRanking',False,'rankHidden')
 a.label('done');a.ret()

def target_cell(a):
 # Native TargetNone ignores extra fields, so the marker is safe even if this
 # optional renderer cannot attach. Native row values/actions stay unchanged.
 a.reg(4);a.store(0);a.missing(0,'done');path(a,4,'display');a.store(0);a.reg(0);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('done',0xB8)
 a.string(',');a.number(1);a.reg(0);a.string('split');a.raw(0x52);a.store(0)
 a.reg(0);a.number(0);a.raw(0x4E);a.string('TargetNone');a.raw(0x66);a.branch('done',0xB8)
 a.reg(0);a.number(1);a.raw(0x4E);a.string('BTFScouted');a.raw(0x66);a.branch('done',0xB8)
 a.reg(0);a.number(2);a.raw(0x4E);a.raw(0x4A);a.store(0)
 for percentage in range(10,101,10):
  a.reg(0);a.number(percentage);a.raw(0x66);a.branch('icon'+str(percentage))
 a.branch('done',0x99)
 for percentage in range(10,101,10):
  a.label('icon'+str(percentage));name='ASSETID_DYNAS_GENERAL_SCOUTING' if percentage==100 else 'ASSETID_DYNAS_GENERAL_SCOUTING_'+str(percentage)
  a.global_object('shared_enum','fblibraries','global','DynastyIcons',name);a.store(0);a.branch('apply',0x99)
 a.label('apply');path(a,1,'mcIcon');a.store(3);a.missing(3,'done');a.set_field(1,'mApplyHighlight',False)
 a.string('AssetLibrary');a.global_object('shared_enum','fblibraries','global','DynastyIcons','LIBRARY_ID');a.number(1);a.raw(0x43);a.number(1);a.reg(3);a.call('SetExtraInfo')
 a.reg(0);a.number(1);a.reg(3);a.call('SetValue')
 a.reg(3);a.number(1);a.global_object('cafe','utility','ColorUtil');a.call('ResetColor')
 for coordinate,dimension,getter in [('_x','mWidth','GetWidth'),('_y','mHeight','GetHeight')]:
  a.reg(3);a.string(coordinate);path(a,1,dimension);a.number(2);a.raw(0x0D);a.number(0);a.reg(3);a.string(getter);a.raw(0x52);a.number(2);a.raw(0x0D,0x0B,0x4F)
 a.label('done');a.ret()

def small_card(a):
 path(a,1,'mData');a.store(9);a.missing(9,'smallDone');path(a,9,'ExtraInfo');a.store(9);a.missing(9,'smallDone')
 a.grade_scale();a.store(8)
 # The native card has its own value conversion. Compare the verified bar
 # values relative to the leader; never depend on a fractional DTO marker.
 a.number(0);a.store(7)
 for i in range(5):
  tag='maximum'+str(i);path(a,9,'SchoolSlot'+str(i),'value');a.raw(0x4A);a.store(10)
  a.reg(10);a.raw(0x44);a.string('number');a.raw(0x66);a.branch(tag,0xB8)
  a.reg(10);a.reg(10);a.raw(0x66);a.branch(tag,0xB8)
  a.reg(10);a.number(0);a.raw(0x48);a.branch(tag)
  a.reg(10);a.number(1000000);a.raw(0x67);a.branch(tag)
  a.reg(10);a.reg(7);a.raw(0x67);a.branch(tag,0xB8);a.reg(10);a.store(7);a.label(tag)
 # Restore native geometry and visibility when this same widget changes modes.
 for name in ['mcTopSlider','mcSlotDivider','mcSlotDividerDotted','mcNumberLabel']:
  path(a,1,name);a.store(2);a.missing(2,'skip'+name)
  path(a,2,'mBTFSmallVisible');a.raw(0x76,0x49);a.branch('saved'+name,0xB8)
  a.reg(2);a.string('mBTFSmallVisible');a.reg(2);a.member('_visible');a.raw(0x4F)
  a.label('saved'+name);a.reg(8);a.number(0);a.raw(0x66);a.branch('show'+name)
  if name!='mcTopSlider':a.reg(8);a.number(2);a.raw(0x66);a.branch('show'+name)
  a.set_field(2,'_visible',False);a.branch('skip'+name,0x99)
  a.label('show'+name);a.reg(2);a.string('_visible');path(a,2,'mBTFSmallVisible');a.raw(0x4F)
  a.label('skip'+name)
 for i in range(5):
  tag=str(i);rank='mcTopNumber'+tag;bar='mcInfluenceSlider'+tag
  path(a,1,rank);a.store(2);a.missing(2,'done'+tag)
  path(a,1,bar);a.store(3);a.missing(3,'done'+tag)
  a.reg(8);a.number(0);a.raw(0x66);a.branch('restore'+tag)
  path(a,2,'mBTFSmallX');a.raw(0x76,0x49);a.branch('geometrySaved'+tag,0xB8)
  for field,source in [('mBTFSmallX','_x'),('mBTFSmallWidth','_width'),('mBTFSmallVisible','_visible')]:
   a.reg(2);a.string(field);a.reg(2);a.member(source);a.raw(0x4F)
  a.reg(3);a.string('mBTFSmallVisible');a.reg(3);a.member('_visible');a.raw(0x4F)
  a.label('geometrySaved'+tag);a.reg(8);a.number(2);a.raw(0x66);a.branch('rankOnly'+tag);a.set_field(3,'_visible',False)
  a.reg(8);a.number(3);a.raw(0x66);a.branch('blind'+tag)
  for committed in ['IsCommited','IsSigned']:
   path(a,9,committed);a.raw(0x73,0x66);a.branch('empty'+tag)
  path(a,9,'SchoolSlot'+tag);a.store(5);a.missing(5,'empty'+tag)
  path(a,5,'IsEmpty');a.raw(0x73,0x66);a.branch('empty'+tag)
  path(a,5,'value');a.raw(0x4A);a.store(6)
  a.reg(6);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('unknown'+tag,0xB8)
  a.reg(6);a.reg(6);a.raw(0x66);a.branch('unknown'+tag,0xB8)
  a.reg(6);a.number(0);a.raw(0x48);a.branch('unknown'+tag)
  a.reg(6);a.number(1000000);a.raw(0x67);a.branch('unknown'+tag)
  a.reg(7);a.number(0);a.raw(0x67);a.branch('unknown'+tag,0xB8)
  a.reg(7);a.reg(6);a.raw(0x0B);a.number(100);a.raw(0x0C);a.reg(7);a.raw(0x0D);a.store(10)
  for edge,value in [(10,90),(25,70),(50,50),(80,30)]:
   a.reg(10);a.number(edge);a.raw(0x48);a.branch('label'+tag+'_'+str(value))
  a.branch('label'+tag+'_10',0x99)
  a.label('unknown'+tag);a.string('INTEREST UNKNOWN');a.store(6);a.branch('position'+tag,0x99)
  for value,label in [(90,'FAVORITE'),(70,'CONTENDER'),(50,'IN THE MIX'),(30,'OUTSIDE LOOKING IN'),(10,'LONG SHOT')]:
   a.label('label'+tag+'_'+str(value));a.string(label);a.store(6);a.branch('position'+tag,0x99)
  a.label('position'+tag);a.set_field(2,'_visible',True)
  a.reg(2);a.string('_x');a.reg(3);a.member('_x');a.raw(0x4F)
  a.number(0);a.reg(3);a.string('GetWidth');a.raw(0x52);a.store(4)
  a.reg(4);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('fallbackWidth'+tag,0xB8)
  a.reg(4);a.number(24);a.raw(0x48);a.branch('fallbackWidth'+tag);a.reg(4);a.number(512);a.raw(0x67);a.branch('fallbackWidth'+tag);a.branch('widthReady'+tag,0x99)
  a.label('fallbackWidth'+tag);a.number(150);a.store(4);a.label('widthReady'+tag)
  # Keep an unchanged draw free of text-style allocations.
  for field,value in [('resize','RESIZE_SCALEDOWN'),('hAlign','HALIGN_LEFT'),('maxNumLines',1),('resizeWidth',None)]:
   path(a,2,'mData','ExtraInfo',field)
   if value is None:a.reg(4)
   elif isinstance(value,str):a.global_object('art_styleguide','Font',value)
   else:a.number(value)
   a.raw(0x66);a.branch('style'+tag,0xB8)
  a.branch('labelReady'+tag,0x99);a.label('style'+tag)
  a.string('resize');a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN');a.string('hAlign');a.global_object('art_styleguide','Font','HALIGN_LEFT');a.string('resizeWidth');a.reg(4);a.string('maxNumLines');a.number(1);a.number(4);a.raw(0x43);a.number(1);a.reg(2);a.call('SetExtraInfo')
  a.label('labelReady'+tag);path(a,2,'mData','display');a.reg(6);a.raw(0x66);a.branch('done'+tag)
  a.reg(6);a.number(1);a.reg(2);a.call('SetLabel');a.branch('done'+tag,0x99)
  a.label('blind'+tag);a.set_field(2,'_visible',False)
  for arrow in ['mcArrowUp'+tag,'mcArrowDown'+tag]:visible(a,arrow,False,'hidden'+arrow)
  a.branch('done'+tag,0x99);a.label('empty'+tag);a.set_field(2,'_visible',False);a.branch('done'+tag,0x99)
  a.label('rankOnly'+tag);a.set_field(3,'_visible',False);a.branch('restoreGeometry'+tag,0x99)
  a.label('restore'+tag);path(a,3,'mBTFSmallVisible');a.raw(0x76,0x49);a.branch('restoreGeometry'+tag)
  a.reg(3);a.string('_visible');path(a,3,'mBTFSmallVisible');a.raw(0x4F)
  a.label('restoreGeometry'+tag);path(a,2,'mBTFSmallX');a.raw(0x76,0x49);a.branch('done'+tag)
  a.reg(2);a.string('_x');path(a,2,'mBTFSmallX');a.raw(0x4F)
  a.reg(2);a.string('_visible');path(a,2,'mBTFSmallVisible');a.raw(0x4F)
  a.string('resize');a.global_object('art_styleguide','Font','RESIZE_NONE');a.string('hAlign');a.global_object('art_styleguide','Font','HALIGN_CENTER');a.string('resizeWidth');path(a,2,'mBTFSmallWidth');a.number(3);a.raw(0x43);a.number(1);a.reg(2);a.call('SetExtraInfo')
  a.label('done'+tag)
 a.label('smallDone');a.ret()

def progress(a):
 a.grade_scale();a.number(0);a.raw(0x66);a.branch('restore')
 for i,widget in enumerate(['mcTopSlider','mcTopSliderMask']):
  path(a,1,widget);a.store(2);a.missing(2,'skip'+str(i));path(a,1,'mBTFProgressVisible'+str(i));a.raw(0x76,0x49);a.branch('saved'+str(i),0xB8)
  a.reg(1);a.string('mBTFProgressVisible'+str(i));a.reg(2);a.member('_visible');a.raw(0x4F)
  a.label('saved'+str(i));a.set_field(2,'_visible',False);a.label('skip'+str(i))
 a.branch('done',0x99);a.label('restore')
 for i,widget in enumerate(['mcTopSlider','mcTopSliderMask']):
  path(a,1,widget);a.store(2);a.missing(2,'restoreSkip'+str(i));path(a,1,'mBTFProgressVisible'+str(i));a.store(3);a.reg(3);a.raw(0x76,0x49);a.branch('restoreSkip'+str(i));a.reg(2);a.string('_visible');a.reg(3);a.raw(0x4F);a.label('restoreSkip'+str(i))
 a.label('done');a.ret()

def cutoff(a):
 # Runs after the verified native Overview._PopulateTopSchoolsList body.
 # Remove only the presentation divider; do not change school rows, locked-out
 # notices, recruit data, advancement or native ranking/cutoff decisions.
 a.grade_scale();a.number(1);a.raw(0x66);a.branch('filter')
 a.grade_scale();a.number(3);a.raw(0x66);a.branch('done',0xB8)
 a.label('filter');path(a,1,'mTopSchoolItemProvider');a.store(2);a.missing(2,'done')
 a.number(0);a.reg(2);a.string('GetNumItems');a.raw(0x52);a.store(3)
 a.reg(3);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('done',0xB8)
 a.reg(3);a.number(0);a.raw(0x67);a.branch('done',0xB8)
 a.reg(3);a.number(32);a.raw(0x67);a.branch('done')
 a.label('next');a.reg(3);a.number(1);a.raw(0x0B);a.store(3)
 a.reg(3);a.number(0);a.raw(0x48);a.branch('done')
 a.reg(3);a.number(1);a.reg(2);a.string('GetItemData');a.raw(0x52);a.store(4);a.missing(4,'next')
 a.reg(4);a.member('display');a.global_object('loc','career','Recruiting','PROJECTED_CUTOFF');a.raw(0x66);a.branch('next',0xB8)
 a.reg(3);a.number(1);a.reg(2);a.call('RemoveItem');a.branch('next',0x99)
 a.label('done');a.ret()

def coarse_sort(a):
 a.grade_scale();a.number(1);a.raw(0x66);a.branch('coarse')
 a.grade_scale();a.number(3);a.raw(0x66);a.branch('fallback')
 a.branch('native',0x99)
 a.label('coarse');a.reg(2);a.member('BTFStandingSort');a.raw(0x4A);a.store(0)
 a.reg(1);a.member('BTFStandingSort');a.raw(0x4A);a.reg(0);a.raw(0x0B);a.store(0)
 a.reg(0);a.number(0);a.raw(0x66);a.branch('fallback');a.reg(0);a.raw(0x3E)
 a.label('fallback');a.reg(2);a.member('NationalRank');a.reg(1);a.member('NationalRank');a.raw(0x0B,0x3E)
 a.label('native');a.pad_code()

def emit(lines,p,offset,argc,regs,flags,size,digest,copyat,copybytes):
 code='{'+','.join(hex(b) for b in p.code)+'}'
 lines.append(f'result.push_back({{{offset},{argc},{regs},{flags},{size},{digest}ull,{max(regs,4) if argc==0 and flags!=26 else regs},{copyat},{copybytes},{code}}});')
 for at in p.scale_offsets:lines.append(f'result.back().scaleOffsets.push_back({at});')
 for at,text in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(text)+f');memcpy(result.back().code.data()+{at},&pointer,8);}}')

def generate(asset,output):
 sys.path.insert(0,r'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools');from inspect_tb_ast import load
 apt=load(asset);lines=['// Generated custom recruiting presentation only; native bodies stay external.','#pragma once','#include "RecruitNotesUiPrograms.h"','#include "CurrencyTopbarUiPrograms.h"','namespace recruitingfog::interestui {']
 for name,body,size,argc,regs,flags,builder,prefix in [('rows',0x36c78,1036,0,4,25,interest_row,False),('progress',0x32f98,2540,0,20,25,progress,False),('cutoff',0x9a38,748,0,27,41,cutoff,False),('rankSort',0x7c18,14,2,3,42,coarse_sort,True),('interestSort',0x7d30,50,2,3,42,coarse_sort,True)]:
  original=apt.data[body:body+size];a=Assembler()
  if prefix:builder(a);copyat=len(a.code);a.code.extend(bytes(size));copybytes=size
  else:copyat=0;copybytes=size;a.code.extend(bytes(size));builder(a)
  p=a.finish();lines.append(f'inline std::vector<notesui::Program> {name}(){{std::vector<notesui::Program> result;');emit(lines,p,body-48,argc,regs,flags,size,fnv(original),copyat,copybytes);lines.append('return result;}')
 # Compose the existing currency adapter rather than replacing or disabling it.
 a=Assembler();scout_row(a);p=a.finish();lines.append('inline std::vector<notesui::Program> scouts(){auto all=currencytopbarui::recruitingPrograms();std::vector<notesui::Program> result;for(auto old:all)if(old.offset=='+str(0x2c548-48)+'){')
 lines.append('if(!old.scaleOffsets.empty()||!old.teamScaleOffsets.empty())throw std::runtime_error("Currency row acquired grade-scale controls");if(old.code.size()<2||old.code[old.code.size()-2]!=0x76||old.code.back()!=0x3e)throw std::runtime_error("Unexpected currency suffix ending");old.code.back()=0x17;const auto start=old.code.size();')
 lines.append('const unsigned char suffix[]={'+','.join(hex(b) for b in p.code)+'};old.code.insert(old.code.end(),std::begin(suffix),std::end(suffix));')
 # Appending must be 8-byte aligned because suffix literal positions are absolute.
 lines[-2]=lines[-2].replace('const auto start=old.code.size();','size_t padding=(8-old.code.size()%8)%8;if(padding==1)padding=9;if(padding%2){old.code.push_back(0x59);old.code.push_back(0x50);old.code.push_back(0x17);padding-=3;}for(;padding;padding-=2){old.code.push_back(0x76);old.code.push_back(0x17);}const auto start=old.code.size();')
 for at,text in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(text)+f');memcpy(old.code.data()+start+{at},&pointer,8);}}')
 lines.append('result.push_back(std::move(old));}return result;}')
 target_asset=asset.parent.parent.parent/'PlayerFog'/'Gaps'/'current-common__ui__art_control__scriptpod.ast'
 target=load(target_asset);body,size,argc,regs,flags=0x5ff50,580,1,5,25
 a=Assembler();a.code.extend(bytes(size));target_cell(a);p=a.finish();lines.append('inline std::vector<notesui::Program> targets(){std::vector<notesui::Program> result;');emit(lines,p,body-48,argc,regs,flags,size,fnv(target.data[body:body+size]),0,size);lines.append('return result;}')
 body,size,argc,regs,flags=0x3bbd0,2241,0,27,25
 a=Assembler();a.code.extend(bytes(size));small_card(a);p=a.finish();lines.append('inline std::vector<notesui::Program> smallCards(){std::vector<notesui::Program> result;');emit(lines,p,body-48,argc,regs,flags,size,fnv(target.data[body:body+size]),0,size);lines.append('return result;}')
 lines.append('}');output.write_text('\n'.join(lines)+'\n',encoding='utf-8');print('Generated 8 isolated recruiting UI banks; existing currency behavior composed.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--asset',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();generate(a.asset,a.output)
