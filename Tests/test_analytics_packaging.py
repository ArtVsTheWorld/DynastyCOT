"""Inspect the packaged Analytics API without loading a DLL or calling game code."""
from pathlib import Path
import importlib.util,json,re,struct,sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'Tools'))
from build_package import analytics_build_id

def exported_api(path):
    data=path.read_bytes();u16=lambda at:struct.unpack_from('<H',data,at)[0];u32=lambda at:struct.unpack_from('<I',data,at)[0]
    pe=u32(0x3c);assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0'
    assert u16(pe+4)==0x8664 and u16(pe+24)==0x20b,'Expected native x64 PE'
    first=pe+24+u16(pe+20);sections=[]
    for i in range(u16(pe+6)):
        at=first+i*40;sections.append((u32(at+12),u32(at+16),u32(at+20)))
    def offset(rva):
        for start,size,raw in sections:
            if start<=rva<start+size:return raw+rva-start
        raise AssertionError('Unmapped native export/data RVA')
    def cstring(at):
        end=data.index(0,at);assert end-at<256;return data[at:end].decode('ascii')
    directory=offset(u32(pe+24+112));functions=offset(u32(directory+28));names=offset(u32(directory+32));ordinals=offset(u32(directory+36))
    for i in range(u32(directory+24)):
        if cstring(offset(u32(names+i*4)))!='DynastyCotAnalyticsApi':continue
        ordinal=u16(ordinals+i*2);assert ordinal<u32(directory+20);rva=u32(functions+ordinal*4);at=offset(rva)
        # MSVC's exact immutable-data getter: lea rax,[rip+disp32]; ret.
        # Refuse a changed getter rather than guessing or executing its code.
        assert data[at:at+3]==b'\x48\x8d\x05' and data[at+7]==0xc3,'Unknown Analytics API export getter'
        api=offset(rva+7+struct.unpack_from('<i',data,at+3)[0]);return struct.unpack_from('<IIQ',data,api)
    raise AssertionError('Packaged Analytics API export missing')

def main():
    checks=0
    def check(ok,message):
        nonlocal checks
        assert ok,message;checks+=1
    sample='20261108010101001';check(analytics_build_id('DynastyEnhancerNative-cot-'+sample+'.dll','DynastyEnhancerRecruitFog-p'+sample+'.dll')==sample,'Matched native source pair rejected')
    for social,fog in [('DynastyEnhancerNative-v103.dll','DynastyEnhancerRecruitFog-v1.dll'),('DynastyEnhancerNative-cot-1.dll','DynastyEnhancerRecruitFog-p2.dll'),('DynastyEnhancerNative-cot-0.dll','DynastyEnhancerRecruitFog-p0.dll'),('DynastyEnhancerNative-cot-18446744073709551616.dll','DynastyEnhancerRecruitFog-p18446744073709551616.dll')]:
        refused=False
        try:analytics_build_id(social,fog)
        except RuntimeError:refused=True
        check(refused,'Unversioned/mixed/invalid native pair accepted')
    stage=Path(sys.argv[1]) if len(sys.argv)>1 else Path(json.loads((ROOT/'Build/current-package.json').read_text())['stage'])
    manifest=json.loads((stage/'package-manifest.json').read_text());expected=analytics_build_id(manifest['nativeInputs']['social']['path'],manifest['nativeInputs']['fog']['path'])
    size,version,build=exported_api(stage/'COT-Runtime/DynastyEnhancer/DynastyEnhancerRecruitFog-v1.dll')
    check((size,version)==(56,1),'Packaged Analytics API ABI differs')
    check(str(build)==expected==manifest['analyticsBridge']['buildId'],'Packaged native API build ID differs from source/manifest')
    check((stage/'COT-Runtime/DynastyEnhancer/DynastyEnhancerNative-v103.dll').is_file(),'Expected stable Social runtime name missing')
    check((stage/'LICENSES/PlayerAnalytics-NOTICES.txt').is_file(),'Analytics notices missing')
    print(f'PASS: {checks} Analytics packaged ABI/build-pair/stable-name/license checks; native DLLs never loaded.')
if __name__=='__main__':main()
