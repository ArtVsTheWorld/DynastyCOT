"""Private .46 native lockout divider fix; preserve the source-built .45 adapters."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import hashlib,json,shutil,subprocess,zipfile

R=Path.cwd()
W=R/"Build/RecruitLockout-20261126"
B=R/"Build/AfterGameUpdate-20261119/BeyondTheFieldPort"
version="1.0.46"
metadata=json.loads((B/"build-integrity.json").read_bytes())
old_sha="158acc02daef6fd1b3ee8f0aec12ca151ae91134bfde49bbd678000e2eb112ee"
def sha(data):return hashlib.sha256(data).hexdigest()
# Verify the latest private .44 binary while retaining the .43 payload's
# complete file manifest; the rebuilt fog incorporates both reviewed layers.
private44=R/"Build/StatsCardPositionFix-20261124"
assert sha((private44/"Managed/BeyondTheField.dll").read_bytes())==json.loads((private44/"build-integrity.json").read_bytes())["dllSha256"]=="033cfbf825262d56520959c0619e5ac72d08de347b181c87d01d67dfe08b78fe"
for name,expected in metadata["sourceInputs"].items():
    assert sha((B/name).read_bytes())==expected,name

native=W/"Native"
shutil.copytree(B/"Native",native,dirs_exist_ok=True)
source=native/"Modules/DynastyEnhancer/Source"
tests=source.parent/"Tests"
changed=[
    "PlayerCardUiPrograms.h","PlayerInGameUiPrograms.h","LiveTables.h","RecruitInterestUiPrograms.h","RecruitNotes.h","RecruitResumeCaptureFields.h",
    "RecruitResumeRuntime.h","RecruitingFeaturesRuntime.h","RecruitingFog.cpp","UiAdapterContracts.generated.h","RecruitInterestResponse.h",
]
for name in changed:
    shutil.copyfile(R/"Modules/DynastyEnhancer/Source"/name,source/name)
for name in ["UniversalUiBindingsTests.cpp","UiDispatchTests.cpp","RecruitInterestUiCompileTests.cpp","RecruitInterestTests.cpp","UiMethodDiscoveryTests.cpp","ReleaseReadinessTests.cpp","RecruitMeasurablesTests.cpp","RecruitingStartupTests.cpp"]:
    shutil.copyfile(R/"Modules/DynastyEnhancer/Tests"/name,tests/name)

vs=Path(subprocess.check_output([r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe",
    "-latest","-products","*","-requires","Microsoft.VisualStudio.Component.VC.Tools.x86.x64","-property","installationPath"],text=True).strip())
tool=sorted((vs/"VC/Tools/MSVC").iterdir())[-1]
kits=Path(r"C:\Program Files (x86)\Windows Kits\10")
sdk=sorted(p for p in (kits/"Include").iterdir() if (p/"ucrt").is_dir() and (kits/"Lib"/p.name/"um/x64").is_dir())[-1]
cl=tool/"bin/Hostx64/x64/cl.exe"
includes=[tool/"include",*(sdk/x for x in ("ucrt","shared","um","winrt")),source]
libs=[tool/"lib/x64",kits/"Lib"/sdk.name/"ucrt/x64",kits/"Lib"/sdk.name/"um/x64"]
common=[str(cl),"/nologo","/MT","/W4","/WX","/EHsc","/std:c++17","/utf-8","/DRECRUIT_NOTES_UI",
    "/DCOT_ANALYTICS_BUILD_ID=202611101030ull",*(f"/I{x}" for x in includes)]
link=["/link",*(f"/LIBPATH:{x}" for x in libs),"/INCREMENTAL:NO"]
log_dir=W/"Verification"
log_dir.mkdir(exist_ok=True)
shutil.copytree(R/"Build/StatsCardPositionFix-20261124/Fixtures",W/"Fixtures",dirs_exist_ok=True)
def run(name,args):
    p=subprocess.run([str(x) for x in args],cwd=W,capture_output=True,text=True)
    (log_dir/(name+".txt")).write_text(p.stdout+p.stderr,encoding="utf-8")
    if p.returncode:raise RuntimeError(name+"\n"+p.stdout[-4000:]+p.stderr[-4000:])
    print(name,"PASS",flush=True)

def verify_native(name,args):
    exe=W/(name+".exe")
    run(name+"-build",[*common,"/Od","/bigobj",tests/(name+".cpp"),f"/Fe:{exe}",*link])
    run(name,[exe,*args])

fog=W/"BeyondTheFieldRecruitFog-v1.dll"
suites={
    "UniversalUiBindingsTests":[W/"Fixtures"],
    "UiDispatchTests":[],
    "RecruitInterestUiCompileTests":[],
    "RecruitInterestTests":[],
    "UiMethodDiscoveryTests":[R/"Build/UiDiscovery/Fixtures"],
    "RecruitMeasurablesTests":[],
    "RecruitingStartupTests":[],
    "ReleaseReadinessTests":[],
}
with ThreadPoolExecutor(max_workers=3) as pool:
    jobs=[pool.submit(run,"native-fog",[*common,"/O2","/LD",source/"RecruitingFog.cpp",f"/Fe:{fog}",*link])]
    jobs.extend(pool.submit(verify_native,name,args) for name,args in suites.items())
    for j in jobs:j.result()

with zipfile.ZipFile(B/"Managed/payload.zip") as z:
    payload={name:z.read(name) for name in z.namelist()}
assert {name:sha(data) for name,data in payload.items()}==metadata["payload"]
entry="COT-Runtime/DynastyEnhancer/BeyondTheFieldRecruitFog-v1.dll"
payload[entry]=fog.read_bytes()
out=W/"Managed"
out.mkdir(exist_ok=True)
archive=out/"payload.zip"
with zipfile.ZipFile(archive,"w",zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for name,data in sorted(payload.items()):
        z.writestr(name,data)
hashes={n:sha(b) for n,b in payload.items()}
manifest=out/"payload.json"
manifest.write_text(json.dumps(hashes,indent=2),encoding="utf-8")
prefix="COT-Runtime/DynastyEnhancer/"
helper_manifest=out/"BeyondTheField.payload-sha256.txt"
helper_manifest.write_text("".join(f"{hashes[n]}|{n[len(prefix):]}\n" for n in sorted(hashes)
    if n.startswith(prefix) and n!=prefix+"beyond-the-field.json"),encoding="utf-8")

managed=[]
for name in ("Identity.cs","StandaloneEntry.cs","BeyondTheFieldAdapter.cs"):
    p=out/name
    data=(B/"Managed"/name).read_bytes()
    if name!="BeyondTheFieldAdapter.cs":
        assert b"1.0.43" in data
        data=data.replace(b"1.0.43",version.encode())
    p.write_bytes(data)
    managed.append(p)
managed += [R/name for name in ("Core/UserStorage.cs","Packaging/Standalone/StandaloneRuntime.cs",
    "Modules/GamedayAtmosphere/Source/ManagedRuntimeLoader.cs","Modules/GamedayAtmosphere/Source/LaunchDiagnostics.cs")]
mmc=Path(r"C:\Apps\CFB 27\MMC\MMC_ModManager_v1.1.0.6")
dll=out/"BeyondTheField.dll"
run("managed",[r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe","/nologo","/target:library","/platform:x64",
    "/optimize+","/warnaserror+","/define:COMBINED_DYNASTY_SETTINGS;COT_BRANDED_RUNTIME",f"/out:{dll}",
    f"/reference:{mmc/'FrostyCore.dll'}",f"/reference:{mmc/'FrostySdk.dll'}","/reference:System.Web.Extensions.dll",
    "/reference:System.IO.Compression.dll","/reference:System.IO.Compression.FileSystem.dll",f"/resource:{archive},Standalone.payload.zip",
    f"/resource:{manifest},Standalone.payload.json",f"/resource:{helper_manifest},BeyondTheField.payload-sha256.txt",*managed])
assert [key for key in hashes if hashes[key]!=metadata["payload"][key]]==[entry]
(W/"build-integrity.json").write_text(json.dumps({"version":version,"dllSha256":sha(dll.read_bytes()),
    "fogSha256":hashes[entry],"baselineDllSha256":old_sha,"changedPayload":[entry],
    "verification":list(suites),"sourceHashes":{n:sha((source/n).read_bytes()) for n in changed},
    "published":False,"installed":False},indent=2)+"\n",encoding="utf-8")
print("Private source-built DLL:",sha(dll.read_bytes()))
