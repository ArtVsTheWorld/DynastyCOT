from pathlib import Path
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT')
p=root/'Tools/generate_practice_contract.py';s=p.read_text().replace("+'}}};\\n')","+'}};\\n')");p.write_text(s,encoding='utf-8',newline='\n')
p=root/'Packaging/Install.ps1';s=p.read_text().replace('[IO.File]::Replace($temporary,$Path,$null)',"[IO.File]::Replace($temporary,$Path,($Path+'.previous'))");p.write_text(s,encoding='utf-8',newline='\n')
