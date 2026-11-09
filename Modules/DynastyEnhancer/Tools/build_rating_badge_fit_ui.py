"""Fit descriptive OVR/OFF/DEF text to the visible native circle, not the text slot."""
def fit_badge(a):
    from build_player_card_ui import path
    done='badgeCircleFitDone'
    for members in [('mData',),('mData','ExtraInfo')]:
        path(a,1,*members);a.raw(0x76,0x49);a.branch(done)
    path(a,1,'mData','ExtraInfo','DETeamRating');a.raw(0x73,0x66);a.branch('badgeCircleTeam')
    a.set_field(1,'mDEActiveRatingTeam',False);a.branch('badgeCircleScale',0x99)
    a.label('badgeCircleTeam');a.set_field(1,'mDEActiveRatingTeam',True)
    a.label('badgeCircleScale');a.grade_scale('auto');a.number(3);a.raw(0x66);a.branch(done,0xb8)
    path(a,1,'mcValue');a.store(4);a.missing(4,done)
    for members in [('mData',),('mData','ExtraInfo')]:
        path(a,4,*members);a.raw(0x76,0x49);a.branch(done)
    path(a,4,'mData','display');a.store(2)
    for word in ['POOR','FAIR','GOOD','GREAT','ELITE','Poor','Fair','Good','Great','Elite']:
        a.reg(2);a.string(word);a.raw(0x66);a.branch('badgeCircleFind')
    a.branch(done,0x99)
    a.label('badgeCircleFind')
    for i,name in enumerate(['mcOutlineExtraSmall','mcOutlineSmall','mcOutlineMedium','mcOutlineLarge','mcOutlineImpact']):
        next_='badgeCircleNext'+str(i)
        path(a,1,name);a.store(2);a.missing(2,next_)
        path(a,2,'_visible');a.raw(0x73,0x66);a.branch(next_,0xb8)
        path(a,2,'_width');a.store(3)
        a.reg(3);a.raw(0x44);a.string('number');a.raw(0x66);a.branch(next_,0xb8)
        a.reg(3);a.reg(3);a.raw(0x66);a.branch(next_,0xb8)
        a.reg(3);a.number(32);a.raw(0x48);a.branch(next_)
        a.reg(3);a.number(512);a.raw(0x67);a.branch(next_)
        a.reg(3);a.number(74);a.raw(0x0c);a.number(100);a.raw(0x0d);a.store(3);a.branch('badgeCircleBound',0x99)
        a.label(next_)
    a.branch(done,0x99)
    a.label('badgeCircleBound')
    # Parent display writers reuse this cache. Do not let their wider text
    # slot undo the native circle bound on a deferred redraw.
    a.reg(4);a.string('mDERatingWidth');a.reg(3);a.raw(0x4f)
    for field in ['resize','resizeWidth','maxNumLines','hAlign']:
        path(a,4,'mData','ExtraInfo',field)
        if field=='resize':a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN')
        elif field=='resizeWidth':a.reg(3)
        elif field=='maxNumLines':a.number(1)
        else:a.global_object('art_styleguide','Font','HALIGN_CENTER')
        a.raw(0x66);a.branch('badgeCircleApply',0xb8)
    a.branch(done,0x99);a.label('badgeCircleApply')
    a.string('resize');a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN')
    a.string('resizeWidth');a.reg(3);a.string('maxNumLines');a.number(1)
    a.string('hAlign');a.global_object('art_styleguide','Font','HALIGN_CENTER');a.number(4);a.raw(0x43)
    a.number(1);a.reg(4);a.call('SetExtraInfo')
    a.label(done)
