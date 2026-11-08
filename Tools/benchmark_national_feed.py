"""Compare identical private feed inputs; never connect to or modify the game."""
from pathlib import Path
import argparse,datetime,hashlib,json,shutil,subprocess
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--feed',type=Path,required=True);parser.add_argument('--baseline',default='68137e6');args=parser.parse_args()
    feed=args.feed.resolve();digest=lambda:hashlib.sha256(feed.read_bytes()).hexdigest();before_hash=digest()
    output=ROOT/'Build'/('NationalBenchmark-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));output.mkdir(parents=True)
    baseline=output/'Baseline';(baseline/'Source').mkdir(parents=True);(baseline/'Vendor').mkdir()
    for name in ['FeedData.h','FeedRefresh.h']:(baseline/'Source'/name).write_bytes(subprocess.check_output(['git','show',args.baseline+':Modules/DynastyEnhancer/Source/'+name],cwd=ROOT))
    shutil.copyfile(ROOT/'Modules/DynastyEnhancer/Vendor/json.hpp',baseline/'Vendor/json.hpp')
    vs=Path(subprocess.check_output([r'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
    tool=sorted((vs/'VC/Tools/MSVC').iterdir())[-1];kits=Path(r'C:/Program Files (x86)/Windows Kits/10');sdk=sorted((kits/'Include').iterdir())[-1]
    flags=['/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8','/I'+str(tool/'include')]+['/I'+str(sdk/x) for x in ['ucrt','shared','um']]
    libraries=['/LIBPATH:'+str(tool/'lib/x64'),*['/LIBPATH:'+str(kits/'Lib'/sdk.name/x) for x in ['ucrt/x64','um/x64']],'/INCREMENTAL:NO']
    source=(ROOT/'Modules/DynastyEnhancer/Tests/NationalFeedBenchmark.cpp').read_text();report={'inputSha256':before_hash,'baseline':args.baseline,'scope':'offline selection/decode; not live menu latency or FPS','samplesPerEdition':50}
    for label,header in [('before',baseline/'Source/FeedData.h'),('after',ROOT/'Modules/DynastyEnhancer/Source/FeedData.h')]:
        cpp=output/(label+'.cpp');cpp.write_text('#define COT_FEED_HEADER "'+header.as_posix()+'"\n'+source);exe=output/(label+'.exe')
        result=subprocess.run([str(tool/'bin/Hostx64/x64/cl.exe'),*flags,str(cpp),'/Fe:'+str(exe),'/link',*libraries],cwd=output,capture_output=True,text=True)
        (output/(label+'-build.log')).write_text(result.stdout+result.stderr);result.check_returncode()
        report[label]=json.loads(subprocess.check_output([str(exe),str(feed)],text=True))
    assert digest()==before_hash,'Read-only input changed during benchmark'
    (output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
