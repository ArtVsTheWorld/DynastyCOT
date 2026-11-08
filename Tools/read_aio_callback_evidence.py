"""Opt-in, read-only inspection of loaded AIO callback fingerprints; no patches."""
from pathlib import Path
import argparse,ctypes as C,json,struct,sys
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--pid',type=int,required=True)
 p.add_argument('--spec',type=Path,default=ROOT/'Modules/DynastyEnhancer/Evidence/Aio141/method-map.json')
 p.add_argument('--out',type=Path,default=ROOT/'Evidence/AioCallbacks');a=p.parse_args()
 a.out=a.out.resolve()
 if not a.out.is_relative_to((ROOT/'Evidence').resolve()):raise ValueError('Captures must stay in ignored COT evidence')
 k=C.WinDLL('kernel32',use_last_error=True)
 class Mbi(C.Structure):_fields_=[('base',C.c_void_p),('allocation',C.c_void_p),('allocationProtect',C.c_uint32),('partition',C.c_uint16),('size',C.c_size_t),('state',C.c_uint32),('protect',C.c_uint32),('type',C.c_uint32)]
 k.OpenProcess.argtypes=[C.c_uint32,C.c_int,C.c_uint32];k.OpenProcess.restype=C.c_void_p
 k.ReadProcessMemory.argtypes=[C.c_void_p,C.c_void_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t)]
 k.VirtualQueryEx.argtypes=[C.c_void_p,C.c_void_p,C.POINTER(Mbi),C.c_size_t];k.VirtualQueryEx.restype=C.c_size_t
 k.CloseHandle.argtypes=[C.c_void_p]
 handle=k.OpenProcess(0x1010,False,a.pid)
 if not handle:raise OSError(C.get_last_error(),'read-only OpenProcess')
 def read(at,n):
  b=C.create_string_buffer(n);copied=C.c_size_t()
  return b.raw if k.ReadProcessMemory(handle,at,b,n,C.byref(copied)) and copied.value==n else b''
 out=a.out;out.mkdir(parents=True,exist_ok=True);rows=json.loads(a.spec.read_text());found=[]
 try:
  at=0
  for count in range(100000):
   m=Mbi()
   if not k.VirtualQueryEx(handle,at,C.byref(m),C.sizeof(m)) or not m.size:break
   base=m.base or 0;at=base+m.size
   if m.state!=0x1000 or m.protect&0x101 or read(base,13)[:11]!=b'Apt Data:1:':continue
   for i,r in enumerate(rows):
    h=read(base+r['new'],48)
    if len(h)!=48 or struct.unpack_from('<IHH',h,8)!=(r['argc'],r['registers'],r['flags']) or struct.unpack_from('<I',h,24)[0]!=r['size']:continue
    body=read(base+r['new']+48,r['size'])
    if not body:continue
    rel=f'callback-{len(found)}.bin';(out/rel).write_bytes(body)
    found.append(dict(index=i,base=base,file=rel,header=h.hex(),name=r['name'],group=r['group'],offset=r['new']))
 finally:k.CloseHandle(handle)
 (out/'index.json').write_text(json.dumps(dict(pid=a.pid,readOnly=True,callbacks=found),indent=2)+'\n')
 print('Read-only callback samples:',len(found),'private evidence only; no process writes.')
if __name__=='__main__':main()
