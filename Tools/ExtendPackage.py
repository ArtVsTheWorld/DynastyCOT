from pathlib import Path
ROOT=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT')
p=ROOT/'Tools/build_package.py';s=p.read_text()
s=s.replace("copy(node,'COT-Runtime/Shared/node.exe','DynastyEnhancer')","copy(node,'COT-Runtime/Shared/node.exe','DynastyEnhancerNews')")
s=s.replace("copy(license_file,'LICENSES/Node-'+node_version+'.txt','DynastyEnhancer')","copy(license_file,'LICENSES/Node-'+node_version+'.txt','DynastyEnhancerNews')")
needle="    adapter=clean_adapter((de/'Source/MmcRecruitingPlugin.cs').read_text())"
s=s.replace(needle,"    copy(ROOT/'Packaging/Install.ps1','Install.ps1','Core')\n    copy(ROOT/'Packaging/README.txt','README.txt','Core')\n"+needle)
needle="    sources.append(emit('EnhancerAdapter.cs',adapter))"
s=s.replace(needle,"    adapter=adapter.replace('var config=new JavaScriptSerializer()', 'DynastyCOT.LaunchAction.VerifyModule(\"DynastyEnhancerNews\");\\n    var config=new JavaScriptSerializer()')\n"+needle)
p.write_text(s,encoding='utf-8',newline='\n')
