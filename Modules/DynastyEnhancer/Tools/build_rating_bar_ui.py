"""Marked native ColorFillSlider colors, with owned restoration on Off/reuse."""
from build_player_card_ui import path,palette

def restore_bucket_focus(a):
    done='bucketHighlightDone'
    for members in [('mData',),('mData','ExtraInfo')]:
        path(a,1,*members);a.raw(0x76,0x49);a.branch(done)
    path(a,1,'mData','ExtraInfo','DEPlayerRating');a.raw(0x73,0x66);a.branch(done,0xb8)
    path(a,1,'mData','ExtraInfo','NumberOfSteps');a.number(20);a.raw(0x66);a.branch(done,0xb8)
    path(a,1,'mcHighlight');a.store(4);a.missing(4,done)
    for register in (1,4):
        path(a,register,'mIsFocused');a.raw(0x44);a.string('boolean');a.raw(0x66);a.branch(done,0xb8)
    path(a,1,'mIsFocused');path(a,4,'mIsFocused');a.raw(0x66);a.branch(done)
    path(a,1,'mIsFocused');a.raw(0x73,0x66);a.branch('bucketHighlightFocus')
    a.global_object('cafe','enum','FocusMethod','JUMP');a.number(1);a.reg(4);a.call('Unfocus');a.branch(done,0x99)
    a.label('bucketHighlightFocus')
    a.global_object('cafe','enum','FocusMethod','JUMP');a.number(1);a.reg(4);a.call('Focus')
    a.label(done)

def rating_bars(a):
    done='ratingBarsDone'
    path(a,1,'mSlider');a.store(4);a.missing(4,done)
    path(a,4,'mData','ExtraInfo');a.store(5);a.missing(5,done)
    path(a,4,'mDECotGradeFillState');a.store(6)
    for parts in [('mData',),('mData','ExtraInfo')]:
        path(a,1,*parts);a.raw(0x76,0x49);a.branch('barRestore')
    path(a,1,'mData','ExtraInfo','DEPlayerRating');a.raw(0x73,0x66);a.branch('barRestore',0xb8)
    path(a,1,'mData','ExtraInfo','NumberOfSteps');a.number(20);a.raw(0x66);a.branch('barRestore')
    a.grade_colors_enabled();a.number(1);a.raw(0x66);a.branch('barRestore',0xb8)
    path(a,5,'RecolorByValue');a.raw(0x44);a.string('boolean');a.raw(0x66);a.branch(done,0xb8)
    path(a,5,'FillColor');a.raw(0x44);a.string('number');a.raw(0x66);a.branch(done,0xb8)
    path(a,1,'mData','ExtraInfo','RightLabelDisplayOverride');a.store(2)
    palette(a,2,3,'barRestore','ratingBarPalette');a.reg(3);a.number(0);a.raw(0x48);a.branch('barRestore')
    a.missing(6,'barNewState')
    path(a,5,'RecolorByValue');a.raw(0x74,0x66);a.branch('barNewState',0xb8)
    path(a,5,'FillColor');path(a,6,'applied');a.raw(0x66);a.branch('barNewState',0xb8)
    a.branch('barApply',0x99)
    a.label('barNewState');a.empty_object(6)
    for prop in ['RecolorByValue','FillColor']:
        a.reg(6);a.string(prop);path(a,5,prop);a.raw(0x4f)
    a.label('barApply');a.reg(6);a.string('applied');a.reg(3);a.raw(0x4f)
    a.reg(4);a.string('mDECotGradeFillState');a.reg(6);a.raw(0x4f)
    path(a,5,'RecolorByValue');a.raw(0x74,0x66);a.branch('barSetter',0xb8)
    path(a,5,'FillColor');a.reg(3);a.raw(0x66);a.branch(done)
    a.label('barSetter');a.string('FillColor');a.reg(3);a.string('RecolorByValue');a.raw(0x74);a.number(2);a.raw(0x43)
    a.number(1);a.reg(4);a.call('SetExtraInfo');a.branch(done,0x99)
    a.label('barRestore');a.missing(6,done);a.empty_object(7);a.number(0);a.store(8)
    for prop in ['RecolorByValue','FillColor']:
        path(a,5,prop)
        if prop=='RecolorByValue':a.raw(0x74)
        else:path(a,6,'applied')
        a.raw(0x66);a.branch('restore'+prop,0xb8)
        a.reg(7);a.string(prop);path(a,6,prop);a.raw(0x4f)
        a.reg(8);a.raw(0x50);a.store(8);a.label('restore'+prop)
    a.reg(8);a.number(0);a.raw(0x67);a.branch('barClear',0xb8)
    a.reg(7);a.number(1);a.reg(4);a.call('SetExtraInfo')
    a.label('barClear');a.reg(4);a.string('mDECotGradeFillState');a.raw(0x76,0x4f)
    a.label(done)
