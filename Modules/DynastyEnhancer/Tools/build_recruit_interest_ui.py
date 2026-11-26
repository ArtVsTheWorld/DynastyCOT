"""Recruiting-only display suffixes; native bodies verified/copied at runtime."""
from pathlib import Path
import argparse,json,sys,struct
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
 path(a,1,'mData');a.store(3);a.missing(3,'unknown')
 path(a,3,'ExtraInfo');a.store(3);a.missing(3,'unknown')
 path(a,3,'BTFFullStanding');a.store(3)
 a.reg(3);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('unknown',0xB8)
 a.reg(3);a.string('');a.raw(0x66);a.branch('setTier',0xB8)
 a.label('unknown');a.string('INTEREST UNKNOWN');a.store(3)
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
 # Verified native Draw formats RECRUITING_INT + InterestedStatus into
 # mcInterestedStatus. mcUserRanking is the MANUAL board position and must
 # remain entirely native. Keep this policy independent of money and grades.
 a.grade_scale();a.store(0)
 path(a,1,'mData');a.store(3);a.missing(3,'boardExtraMissing')
 path(a,3,'ExtraInfo');a.store(3);a.missing(3,'boardExtraMissing');a.branch('boardExtraReady',0x99)
 a.label('boardExtraMissing');a.empty_object(3);a.label('boardExtraReady')
 for index,widget in enumerate(['mcInterestedStatus','mcUpArrow','mcDownArrow']):
  tag='boardInterest'+str(index);done=tag+'Done';capture=tag+'Capture';hide=tag+'Hide'
  path(a,1,widget);a.store(2);a.missing(2,done)
  a.reg(0);a.number(3);a.raw(0x66);a.branch(hide)
  if index==0:a.reg(0);a.number(1);a.raw(0x66);a.branch(hide)
  # Restore only our own visibility on this exact native widget/data pair.
  path(a,1,tag+'Target');a.reg(2);a.raw(0x66);a.branch(done,0xB8)
  path(a,1,tag+'Data');path(a,1,'mData');a.raw(0x66);a.branch(tag+'Clear',0xB8)
  for field in ['InterestedStatus','RecruitStage','CurrentUserTeamRankChange']:
   path(a,1,tag+field);path(a,3,field);a.raw(0x66);a.branch(tag+'Clear',0xB8)
  a.reg(2);a.string('_visible');path(a,1,tag+'Visible');a.raw(0x4F)
  a.label(tag+'Clear');a.reg(1);a.string(tag+'Target');a.raw(0x76,0x4F);a.branch(done,0x99)
  a.label(hide)
  path(a,1,tag+'Target');a.reg(2);a.raw(0x66);a.branch(capture,0xB8)
  path(a,1,tag+'Data');path(a,1,'mData');a.raw(0x66);a.branch(capture,0xB8)
  for field in ['InterestedStatus','RecruitStage','CurrentUserTeamRankChange']:
   path(a,1,tag+field);path(a,3,field);a.raw(0x66);a.branch(capture,0xB8)
  a.branch(tag+'Captured',0x99)
  a.label(capture)
  a.reg(1);a.string(tag+'Target');a.reg(2);a.raw(0x4F)
  a.reg(1);a.string(tag+'Data');path(a,1,'mData');a.raw(0x4F)
  for field in ['InterestedStatus','RecruitStage','CurrentUserTeamRankChange']:
   a.reg(1);a.string(tag+field);path(a,3,field);a.raw(0x4F)
  a.reg(1);a.string(tag+'Visible');path(a,2,'_visible');a.raw(0x4F)
  a.label(tag+'Captured')
  path(a,2,'_visible');a.raw(0x74,0x66);a.branch(done)
  a.set_field(2,'_visible',False);a.label(done)
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

def lockout_row(a):
 # Native registers 8/3 are the stage cutoff and current row ordinal. Keep
 # the native stage decision; undo only presentation reordering. Locked is
 # the actual native current-school gate, independent of its position.
 path(a,6,'ExtraInfo');a.store(15);a.missing(15,'lockRowDone')
 path(a,1,'mCurrentRecruit');a.store(18);a.missing(18,'lockRowUnknown')
 path(a,15,'TeamId');path(a,2,'TeamId');a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 path(a,18,'RecruitStage');a.raw(0x44);a.string('number');a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(8);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(3);a.store(16)
 a.grade_scale();a.store(17)
 for mode in [1,3]:a.reg(17);a.number(mode);a.raw(0x66);a.branch('lockReadOrder')
 a.branch('lockOrderReady',0x99)
 a.label('lockReadOrder');path(a,2,'BTFNativeSchoolOrder');a.store(16)
 a.reg(16);a.raw(0x44);a.string('undefined');a.raw(0x66);a.branch('lockHasOrder',0xB8)
 # Some native entry routes reach this method without the optional DTO
 # transform. Their list is still native. Only allow that fallback when
 # neither of our reordering markers exists; never guess a lost ordinal.
 for marker in ['BTFStanding','BTFInterestOriginal']:
  path(a,2,marker);a.raw(0x44);a.string('undefined');a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(3);a.store(16);a.branch('lockOrderReady',0x99)
 a.label('lockHasOrder')
 a.reg(16);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(16);a.raw(0x4A);a.store(16)
 a.reg(16);a.reg(16);a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(16);a.raw(0x18);a.reg(16);a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(16);a.number(0);a.raw(0x48);a.branch('lockRowUnknown')
 a.reg(16);a.reg(26);a.raw(0x48);a.branch('lockRowUnknown',0xB8)
 a.label('lockOrderReady');a.set_field(15,'IsBelowCutoff',False)
 path(a,18,'RecruitStage');a.global_object('adapterenum_cfb_dynasty','RecruitingBoardFlow','CFME_RECRUITSTAGE_TOP10');a.raw(0x67);a.branch('lockRowKnown',0xB8)
 a.reg(16);a.reg(8);a.raw(0x48);a.branch('lockCheckOwner')
 a.set_field(15,'IsBelowCutoff',True)
 a.label('lockCheckOwner');path(a,15,'IsCurrentUserTeam');a.raw(0x73,0x66);a.branch('lockRowKnown',0xB8)
 path(a,18,'Locked');a.store(17)
 a.reg(17);a.raw(0x44);a.string('boolean');a.raw(0x66);a.branch('lockRowUnknown',0xB8)
 a.reg(15);a.string('IsBelowCutoff');a.reg(17);a.raw(0x4F)
 a.label('lockRowKnown');a.set_field(15,'BTFNativeCutoffKnown',True);a.branch('lockRowDone',0x99)
 a.label('lockRowUnknown');a.set_field(15,'BTFNativeCutoffKnown',False)
 a.label('lockRowDone')

def lockout_divider(a):
 # This runs only during list population, before it is published to the view.
 # Retain row/header objects and order within each eligibility group. No Draw
 # scanning, DB reads, action changes, rank-derived current-school lockout or
 # new headers in stages where the native screen did not supply one.
 path(a,1,'mTopSchoolItemProvider');a.store(2);a.missing(2,'lockDone')
 a.number(0);a.reg(2);a.string('GetNumItems');a.raw(0x52);a.store(3)
 a.reg(3);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('lockDone',0xB8)
 a.reg(3);a.number(0);a.raw(0x67);a.branch('lockDone',0xB8)
 a.reg(3);a.number(32);a.raw(0x67);a.branch('lockDone')
 for reg in [5,6]:a.number(0);a.raw(0x42);a.store(reg)
 a.raw(0x76);a.store(7);a.number(0);a.store(8);a.number(0);a.store(9);a.raw(0x74);a.store(12)
 a.label('lockGather');a.reg(9);a.reg(3);a.raw(0x48);a.branch('lockGathered',0xB8)
 a.reg(9);a.number(1);a.reg(2);a.string('GetItemData');a.raw(0x52);a.store(4);a.missing(4,'lockDone')
 path(a,4,'display');a.global_object('loc','career','Recruiting','LOCKED_OUT');a.raw(0x66);a.branch('lockHeader')
 path(a,4,'ExtraInfo');a.store(10);a.missing(10,'lockDone')
 path(a,10,'BTFNativeCutoffKnown');a.raw(0x73,0x66);a.branch('lockDone',0xB8)
 path(a,10,'IsBelowCutoff');a.store(11)
 a.reg(11);a.raw(0x44);a.string('boolean');a.raw(0x66);a.branch('lockDone',0xB8)
 a.reg(11);a.branch('lockBelow')
 a.reg(7);a.raw(0x76,0x49);a.branch('lockEligiblePlaced')
 a.raw(0x73);a.store(12);a.label('lockEligiblePlaced')
 a.reg(4);a.number(1);a.reg(5);a.call('push');a.branch('lockNext',0x99)
 a.label('lockBelow');a.reg(7);a.raw(0x76,0x49);a.branch('lockBelowPlaced',0xB8)
 a.raw(0x73);a.store(12);a.label('lockBelowPlaced')
 a.reg(4);a.number(1);a.reg(6);a.call('push');a.branch('lockNext',0x99)
 a.label('lockHeader');a.reg(7);a.raw(0x76,0x49);a.branch('lockDone',0xB8)
 a.reg(4);a.store(7);a.reg(9);a.store(8)
 a.label('lockNext');a.reg(9);a.raw(0x50);a.store(9);a.branch('lockGather',0x99)
 a.label('lockGathered');a.reg(7);a.raw(0x76,0x49);a.branch('lockDone')
 # Unchanged native groups need no provider invalidation or redraw.
 a.reg(12);a.branch('lockCompare')
 path(a,5,'length');a.reg(8);a.raw(0x66);a.branch('lockCompare',0xB8)
 path(a,6,'length');a.number(0);a.raw(0x67);a.branch('lockCompare',0xB8)
 a.branch('lockDone',0x99)
 a.label('lockCompare');a.reg(3);a.store(9)
 a.label('lockClear');a.reg(9);a.number(1);a.raw(0x0B);a.store(9)
 a.reg(9);a.number(0);a.raw(0x48);a.branch('lockAddEligible')
 a.reg(9);a.number(1);a.reg(2);a.call('RemoveItem');a.branch('lockClear',0x99)
 a.label('lockAddEligible');a.number(0);a.store(9)
 for reg,tag in [(5,'Eligible'),(6,'Below')]:
  a.label('lockAdd'+tag+'Loop');a.reg(9);path(a,reg,'length');a.raw(0x48);a.branch('lockAdd'+tag+'Done',0xB8)
  a.reg(reg);a.reg(9);a.raw(0x4E);a.number(1);a.reg(2);a.call('AddItem')
  a.reg(9);a.raw(0x50);a.store(9);a.branch('lockAdd'+tag+'Loop',0x99)
  a.label('lockAdd'+tag+'Done')
  if reg==5:
   path(a,6,'length');a.number(0);a.raw(0x67);a.branch('lockDone',0xB8)
   a.reg(7);a.number(1);a.reg(2);a.call('AddItem');a.number(0);a.store(9)
 a.label('lockDone');a.ret()

# Offsets in the verified Overview._PopulateTopSchoolsList body. Keep the
# native loop and its branch targets, including the exact 748-byte exit.
FULL_OVERVIEW_BODY=0x9a38
FULL_OVERVIEW_SIZE=748
FULL_OVERVIEW_ADD_ROW=0x9d04-FULL_OVERVIEW_BODY
FULL_OVERVIEW_NEXT_ROW=0x9aed-FULL_OVERVIEW_BODY

def valid_school(a,school,value,invalid,team=None):
 # Require a real school identity and a finite, nonnegative total. The source
 # is the input to BOTH native bar segments, before the stage clip is applied.
 a.missing(school,invalid)
 path(a,school,'TeamId');a.store(value)
 a.reg(value);a.raw(0x44);a.string('number');a.raw(0x66);a.branch(invalid,0xB8)
 a.reg(value);a.reg(value);a.raw(0x66);a.branch(invalid,0xB8)
 a.reg(value);a.number(0);a.raw(0x48);a.branch(invalid)
 a.reg(value);a.number(65534);a.raw(0x67);a.branch(invalid)
 a.reg(value);a.number(255);a.raw(0x66);a.branch(invalid)
 if team is not None:
  path(a,team,'TeamId');a.reg(value);a.raw(0x66);a.branch(invalid,0xB8)
 path(a,school,'influenceTotal');a.store(value)
 a.reg(value);a.raw(0x44);a.string('number');a.raw(0x66);a.branch(invalid,0xB8)
 a.reg(value);a.reg(value);a.raw(0x66);a.branch(invalid,0xB8)
 a.reg(value);a.number(0);a.raw(0x48);a.branch(invalid)
 a.reg(value);a.number(1000000);a.raw(0x67);a.branch(invalid)

def full_school(a):
 # Native registers 24/26 hold the active recruit's TopSchools array/count.
 # Register 2 is this exact school, 6 is its newly built provider row, and 3
 # is the loop index. Scratch registers 9--14 are unused by this native body.
 lockout_row(a)
 a.grade_scale();a.number(1);a.raw(0x66);a.branch('addNativeSchool',0xB8)
 a.reg(3);a.number(0);a.raw(0x66);a.branch('fullTier',0xB8)
 a.number(0);a.store(9)
 a.reg(26);a.number(32);a.raw(0x67);a.branch('fullTier')
 a.number(0);a.store(10)
 a.label('fullLeaderLoop')
 a.reg(10);a.reg(26);a.raw(0x48);a.branch('fullTier',0xB8)
 a.reg(24);a.reg(10);a.raw(0x4E);a.store(11)
 valid_school(a,11,12,'fullNextLeader')
 a.reg(12);a.reg(9);a.raw(0x67);a.branch('fullNextLeader',0xB8)
 a.reg(12);a.store(9)
 a.label('fullNextLeader');a.reg(10);a.raw(0x50);a.store(10);a.branch('fullLeaderLoop',0x99)
 a.label('fullTier')
 path(a,6,'ExtraInfo');a.store(13);a.missing(13,'addNativeSchool')
 a.reg(9);a.number(0);a.raw(0x67);a.branch('fullUnknown',0xB8)
 valid_school(a,2,12,'fullUnknown',team=13)
 # Relative gap to the strongest projected total. No stage minimum, clipping
 # or projected-cutoff inference enters these broad display tiers.
 a.reg(9);a.reg(12);a.raw(0x0B);a.number(100);a.raw(0x0C);a.reg(9);a.raw(0x0D);a.store(14)
 for edge in (10,25,50,80):
  a.reg(14);a.number(edge);a.raw(0x67);a.branch('fullGap'+str(edge),0xB8)
 a.string('LONG SHOT');a.branch('fullWrite',0x99)
 a.label('fullUnknown');a.string('INTEREST UNKNOWN');a.branch('fullWrite',0x99)
 for edge,label in ((10,'FAVORITE'),(25,'CONTENDER'),(50,'IN THE MIX'),(80,'OUTSIDE LOOKING IN')):
  a.label('fullGap'+str(edge));a.string(label);a.branch('fullWrite',0x99)
 a.label('fullWrite');a.store(14)
 a.reg(13);a.string('BTFFullStanding');a.reg(14);a.raw(0x4F)
 a.label('addNativeSchool')
 # The verified native epilogue adds the row, increments its native index and
 # jumps back. Run it here so even synchronous first Draw sees the label.
 a.reg(6);a.number(1);path(a,1,'mTopSchoolItemProvider');a.call('AddItem')
 a.reg(3);a.raw(0x50);a.store(3);a.branch('nextNativeSchool',0x99)

def full_overview(a):
 a.code.extend(bytes(FULL_OVERVIEW_ADD_ROW))
 a.branch('fullSchool',0x99)
 a.code.extend(bytes(FULL_OVERVIEW_SIZE-len(a.code)))
 # The existing filter returns; embed it as a function-local suffix with its
 # return removed, then repair the remaining native Locked Out header.
 cutoff(a);assert a.code[-2:]==bytes((0x76,0x3e))
 a.code[-2:]=bytes((0x76,0x17));lockout_divider(a)
 a.label('fullSchool')
 a.labels['nextNativeSchool']=FULL_OVERVIEW_NEXT_ROW
 full_school(a)


FULL_OFFER_HEADER_BODY=0xaae0
FULL_OFFER_HEADER_SIZE=437

def offer_header(a):
 # The verified Overview body already submitted HeaderStrings and
 # ColumnXOffsets to its native widget. Reuse that exact layout, including
 # commitment stage icons. Index four is the existing Offer heading.
 a.grade_scale();a.number(3);a.raw(0x66);a.branch('noOfferTotal')
 path(a,1,'mCurrentRecruit');a.store(0);a.missing(0,'noOfferTotal')
 path(a,0,'BTFOfferTotalCaption');a.store(0)
 a.reg(0);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('noOfferTotal',0xB8)
 path(a,0,'length');a.number(4);a.raw(0x48);a.branch('noOfferTotal')
 path(a,0,'length');a.number(5);a.raw(0x67);a.branch('noOfferTotal')
 path(a,1,'mTopSchoolsListHeaders');a.store(2);a.missing(2,'noOfferTotal')
 path(a,2,'mData');a.store(3);a.missing(3,'noOfferTotal')
 path(a,3,'ExtraInfo');a.store(3);a.missing(3,'noOfferTotal')
 path(a,3,'HeaderStrings');a.reg(5);a.raw(0x66);a.branch('noOfferTotal',0xB8)
 a.reg(5);a.number(4);a.raw(0x4E);a.store(1)
 a.reg(1);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('noOfferTotal',0xB8)
 path(a,1,'length');a.number(6);a.raw(0x67);a.branch('noOfferTotal')
 a.reg(5);a.number(4);a.reg(1);a.reg(0);a.raw(0x47,0x4F)
 a.reg(3);a.number(1);a.reg(2);a.call('SetExtraInfo')
 a.label('noOfferTotal');a.ret()


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
 for name,body,size,argc,regs,flags,builder,prefix in [('rows',0x36c78,1036,0,4,25,interest_row,False),('progress',0x32f98,2540,0,20,25,progress,False),('cutoff',FULL_OVERVIEW_BODY,FULL_OVERVIEW_SIZE,0,27,41,full_overview,False),('offerHeader',FULL_OFFER_HEADER_BODY,FULL_OFFER_HEADER_SIZE,0,6,41,offer_header,False),('rankSort',0x7c18,14,2,3,42,coarse_sort,True),('interestSort',0x7d30,50,2,3,42,coarse_sort,True)]:
  original=apt.data[body:body+size];a=Assembler()
  if prefix:builder(a);copyat=len(a.code);a.code.extend(bytes(size));copybytes=size
  elif name=='cutoff':
   # Verify the exact old tail/exit before rebuilding this small epilogue.
   assert fnv(original)==13559666481607729469
   assert original[FULL_OVERVIEW_ADD_ROW:FULL_OVERVIEW_ADD_ROW+3]==bytes((0xb9,6,0x5a))
   assert FULL_OVERVIEW_SIZE+struct.unpack_from('<i',original,FULL_OVERVIEW_SIZE-4)[0]==FULL_OVERVIEW_NEXT_ROW
   copyat=0;copybytes=FULL_OVERVIEW_ADD_ROW;builder(a)
  else:copyat=0;copybytes=size;a.code.extend(bytes(size));builder(a)
  p=a.finish();lines.append(f'inline std::vector<notesui::Program> {name}(){{std::vector<notesui::Program> result;');emit(lines,p,body-48,argc,regs,flags,size,fnv(original),copyat,copybytes)
  if name=='progress':
   # Same typed owner with native currency/layout additions. Complete source
   # instructions and typed string operands are verified before appending.
   from apt_source_pointers import source_pointers
   extended=load(Path('C:/Apps/CFB 27/MMC/ModComparisons/ZEO101-TeamBuilder/combo/Assets/common/ui/node_cfm/recruitinghub/corenode.ast'))
   off,n,digest=399936,3086,11528656320847582138
   assert struct.unpack_from('<IHH',extended.data,off+8)==(0,20,25) and fnv(extended.data[off+48:off+48+n])==digest
   extra=Assembler();extra.code.extend(bytes(n));progress(extra);ep=extra.finish();emit(lines,ep,off,0,20,25,n,digest,0,n)
   for at,value in source_pointers(extended,off,n,[]):lines.append(f'result.back().sourcePointers.push_back({{{at},{value}}});')
  lines.append('return result;}')
 # Compose the existing currency adapter rather than replacing or disabling it.
 a=Assembler();scout_row(a);p=a.finish();lines.append('inline std::vector<notesui::Program> scouts(){auto all=currencytopbarui::recruitingPrograms();std::vector<notesui::Program> result;for(auto old:all)if(old.offset=='+str(0x2c548-48)+'){')
 lines.append('if(!old.scaleOffsets.empty()||!old.teamScaleOffsets.empty())throw std::runtime_error("Currency row acquired grade-scale controls");if(old.code.size()<2||old.code[old.code.size()-2]!=0x76||old.code.back()!=0x3e)throw std::runtime_error("Unexpected currency suffix ending");old.code.back()=0x17;const auto start=old.code.size();')
 lines.append('const unsigned char suffix[]={'+','.join(hex(b) for b in p.code)+'};old.code.insert(old.code.end(),std::begin(suffix),std::end(suffix));')
 # Appending must be 8-byte aligned because suffix literal positions are absolute.
 lines[-2]=lines[-2].replace('const auto start=old.code.size();','size_t padding=(8-old.code.size()%8)%8;if(padding==1)padding=9;if(padding%2){old.code.push_back(0x59);old.code.push_back(0x50);old.code.push_back(0x17);padding-=3;}for(;padding;padding-=2){old.code.push_back(0x76);old.code.push_back(0x17);}const auto start=old.code.size();')
 for at,text in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(text)+f');memcpy(old.code.data()+start+{at},&pointer,8);}}')
 lines.append('result.push_back(std::move(old));}')
 # Native focus styling redraws this caption after the parent row's Draw.
 # Reapply the same independent visibility policy at both verified boundaries.
 extended=load(Path('C:/Apps/CFB 27/MMC/ModComparisons/ZEO101-TeamBuilder/combo/Assets/common/ui/node_cfm/recruitinghub/corenode.ast'))
 for off,n,digest in [(389056,612,6753933695585567227),(389728,588,4689005749271494068)]:
  assert struct.unpack_from('<IHH',extended.data,off+8)==(1,6,25) and fnv(extended.data[off+48:off+48+n])==digest
  a=Assembler();a.code.extend(bytes(n));scout_row(a);p=a.finish();emit(lines,p,off,1,6,25,n,digest,0,n)
  from apt_source_pointers import source_pointers
  for at,value in source_pointers(extended,off,n,[]):lines.append(f'result.back().sourcePointers.push_back({{{at},{value}}});')
 lines.append('return result;}')
 target_asset=asset.parent.parent.parent/'PlayerFog'/'Gaps'/'current-common__ui__art_control__scriptpod.ast'
 target=load(target_asset);body,size,argc,regs,flags=0x5ff50,580,1,5,25
 a=Assembler();a.code.extend(bytes(size));target_cell(a);p=a.finish();lines.append('inline std::vector<notesui::Program> targets(){std::vector<notesui::Program> result;');emit(lines,p,body-48,argc,regs,flags,size,fnv(target.data[body:body+size]),0,size);lines.append('return result;}')
 body,size,argc,regs,flags=0x3bbd0,2241,0,27,25
 a=Assembler();a.code.extend(bytes(size));small_card(a);p=a.finish();lines.append('inline std::vector<notesui::Program> smallCards(){std::vector<notesui::Program> result;');emit(lines,p,body-48,argc,regs,flags,size,fnv(target.data[body:body+size]),0,size);lines.append('return result;}')
 lines.append('}');output.write_text('\n'.join(lines)+'\n',encoding='utf-8');print('Generated 9 isolated recruiting UI banks; existing currency behavior composed.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--asset',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();generate(a.asset,a.output)
