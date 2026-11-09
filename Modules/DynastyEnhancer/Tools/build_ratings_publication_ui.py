"""Prepare native Ratings presentation rows before publishing them.
Preserve numerical ratings/progression, native bounds and bonus displays.
"""
from build_player_card_ui import path,numeric_grade,palette

def font(a,method):
    a.string('fontColor');a.global_object('art_styleguide','Color','PRIMARY_LIGHT_COLOR');a.number(1);a.raw(0x43)
    a.number(1);a.number(0);a.number(1080);a.number(1);a.global_object('art_styleguide','Font');a.string('GetStyleguide');a.raw(0x52)
    a.string(method);a.raw(0x52);a.string('CloneAndModify');a.raw(0x52)

def prepare(a):
    a.missing(7,'ready');a.number(0);a.store(2)
    a.label('rows');a.reg(2);a.number(128);a.raw(0x48);a.branch('ready',0xb8)
    a.reg(2);path(a,7,'length');a.raw(0x48);a.branch('ready',0xb8)
    a.reg(7);a.reg(2);a.raw(0x4e);a.store(3);a.missing(3,'next')
    a.empty_object(8);path(a,3,'ExtraInfo');a.store(9);a.missing(9,'baseStyles')
    a.reg(9);a.reg(8);a.number(2);a.global_object('cafe','utility','ObjectUtil');a.call('MergeInto')
    a.label('baseStyles')
    for field,method in [('UnfocusTextStyle','GetStyleSUBHEADER_8'),('UnfocusValueTextStyle','GetStyleSPEC_NUMBER_7')]:
        a.reg(8);a.string(field);font(a,method);a.raw(0x4f)
    path(a,3,'value');a.store(4);numeric_grade(a,4,5,'save','prepubGrade')
    palette(a,5,6,'save','prepubPalette')
    a.reg(8);a.string('RightLabelDisplayOverride');a.reg(5);a.raw(0x4f);a.set_field(8,'DEPlayerRating',True)
    a.grade_colors_enabled();a.number(1);a.raw(0x66);a.branch('save',0xb8)
    for field in ['FocusValueTextStyle','UnfocusValueTextStyle','UnfocusableValueTextStyle']:
        path(a,8,field);a.store(9);a.missing(9,'color'+field)
        a.empty_object(0);a.reg(0);a.string('fontColor');a.reg(6);a.raw(0x4f)
        a.reg(8);a.string(field);a.reg(0);a.number(1);a.reg(9);a.string('CloneAndModify');a.raw(0x52,0x4f);a.label('color'+field)
    a.label('save');a.reg(3);a.string('ExtraInfo');a.reg(8);a.raw(0x4f)
    a.label('next');a.reg(2);a.raw(0x50);a.store(2);a.branch('rows',0x99)
    a.label('ready')

def fit_level(a):
    a.number(0);a.number(1);path(a,1,'mSkillLevelDataProvider');a.string('GetItemData');a.raw(0x52);a.store(8);a.missing(8,'fitDone')
    path(a,8,'ExtraInfo');a.store(9);a.missing(9,'fitDone')
    a.number(60);a.store(6)
    path(a,8,'Width');a.store(7);a.reg(7);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('fitBoundReady',0xb8)
    a.reg(7);a.number(16);a.raw(0x48);a.branch('fitBoundReady')
    a.reg(7);a.number(256);a.raw(0x67);a.branch('fitBoundReady')
    a.reg(7);a.store(6);a.label('fitBoundReady')
    for field in ['resize','resizeWidth','maxNumLines','hAlign']:
        path(a,9,field)
        if field=='resize':a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN')
        elif field=='resizeWidth':a.reg(6)
        elif field=='maxNumLines':a.number(1)
        else:a.global_object('art_styleguide','Font','HALIGN_RIGHT')
        a.raw(0x66);a.branch('fitChanged',0xb8)
    a.branch('fitDone',0x99);a.label('fitChanged')
    a.string('resize');a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN')
    a.string('resizeWidth');a.reg(6);a.string('maxNumLines');a.number(1)
    a.string('hAlign');a.global_object('art_styleguide','Font','HALIGN_RIGHT');a.number(4);a.raw(0x43)
    a.number(1);a.reg(9);a.string('CloneAndModify');a.raw(0x52);a.store(10)
    a.reg(10);a.string('ExtraInfo');a.number(0);a.number(3);path(a,1,'mSkillLevelDataProvider');a.call('UpdateProperty')
    a.label('fitDone')


def fit_title(a):
    path(a,1,'mTitleLabel');a.store(8);a.missing(8,'nameFitDone')
    path(a,8,'mData','ExtraInfo');a.store(9);a.missing(9,'nameFitDone')
    for field in ['resize','resizeWidth','maxNumLines']:
        path(a,9,field)
        if field=='resize':a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN')
        elif field=='resizeWidth':a.number(200)
        else:a.number(1)
        a.raw(0x66);a.branch('nameFitChanged',0xb8)
    a.branch('nameFitDone',0x99);a.label('nameFitChanged')
    a.string('resize');a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN')
    a.string('resizeWidth');a.number(200);a.string('maxNumLines');a.number(1);a.number(3);a.raw(0x43)
    a.number(1);a.reg(8);a.call('SetExtraInfo')
    a.label('nameFitDone')
