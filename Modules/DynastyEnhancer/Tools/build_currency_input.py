from build_player_card_ui import path
def adaptive_money(a,done,prefix,format_points):
    a.reg(0);a.raw(0x44);a.string('string');a.raw(0x66);a.branch(prefix+'plain',0xB8)
    a.number(0);a.number(1);a.reg(0);a.string('charAt');a.raw(0x52);a.string('$');a.raw(0x66);a.branch(prefix+'plain',0xB8)
    a.string('$');a.number(1);a.reg(0);a.string('split');a.raw(0x52);a.store(3)
    path(a,3,'length');a.number(2);a.raw(0x66);a.branch(done,0xB8)
    a.reg(3);a.number(1);a.raw(0x4E);a.store(0)
    a.number(1);a.store(2)
    for suffix,multiplier in [('K',1000),('M',1000000)]:
        path(a,0,'length');a.number(1);a.raw(0x0B);a.number(1);a.reg(0);a.string('charAt');a.raw(0x52);a.string(suffix);a.raw(0x66);a.branch(prefix+'not'+suffix,0xB8)
        a.string(suffix);a.number(1);a.reg(0);a.string('split');a.raw(0x52);a.number(0);a.raw(0x4E);a.store(0)
        a.number(multiplier);a.store(2)
        a.label(prefix+'not'+suffix)
    a.string(',');a.number(1);a.reg(0);a.string('split');a.raw(0x52);a.store(3)
    a.string('');a.number(1);a.reg(3);a.string('join');a.raw(0x52);a.store(3)
    path(a,3,'length');a.number(0);a.raw(0x67);a.branch(done,0xB8)
    for invalid in ['NaN','Infinity','-Infinity']:
        a.reg(3);a.string(invalid);a.raw(0x66);a.branch(done)
    a.reg(3);a.raw(0x4A);a.store(0)
    a.reg(0);a.raw(0x4B);a.reg(3);a.raw(0x66);a.branch(prefix+'valid')
    a.reg(0);a.raw(0x4B);a.string('.0');a.raw(0x47);a.reg(3);a.raw(0x66);a.branch(done,0xB8)
    a.label(prefix+'valid');a.reg(0);a.reg(2);a.raw(0x0C);a.number(4000);a.raw(0x0D);a.store(0)
    a.label(prefix+'plain');format_points(a,done,prefix+'base')

