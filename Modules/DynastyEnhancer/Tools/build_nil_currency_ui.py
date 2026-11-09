"""Current NIL only: format the actual slider after each native redraw."""
from build_player_card_ui import path

MARKER='mDECotCurrentNIL'

def mark_current_nil(a):
    path(a,1,'mcCurrentNIL');a.store(0);a.missing(0,'nilCurrencyMarkerDone')
    a.set_field(0,MARKER,True)
    a.label('nilCurrencyMarkerDone')

def geometry(a):
    a.raw(0x76);a.store(6)
    path(a,1,'mData','ExtraInfo','ValueWidth');a.store(7)
    a.reg(7);a.raw(0x44);a.string('number');a.raw(0x66);a.branch('nilCurrencyGeometryDone',0xb8)
    a.reg(7);a.number(1);a.raw(0x67);a.branch('nilCurrencyGeometryDone',0xb8)
    a.reg(7);a.number(4096);a.raw(0x48);a.branch('nilCurrencyGeometryDone',0xb8)
    a.reg(7);a.store(6);a.label('nilCurrencyGeometryDone')

def suffix(a):
    # Reuse the tested bounded, idempotent money-label renderer. The slider's
    # numeric value is read, never replaced, and Off restores only owned styles.
    from build_currency_recruiteditor_ui import suffix as money_label
    for parts in [('mData',),('mData','ExtraInfo')]:
        path(a,1,*parts);a.raw(0x76,0x49);a.branch('done')
    money_label(a,target_path=('mcRightValue',),value_path=('mData','value'),
                state_key='mDECotNilMoney',geometry_builder=geometry,label_register=2)
