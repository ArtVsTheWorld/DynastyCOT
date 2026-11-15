"""Read APT class declarations without recursively decoding method bodies.

Unknown root instructions fail closed. Method bytecode is verified separately;
unrelated nested functions must not hide otherwise valid class declarations.
"""
import struct

def symbols(apt):
    data=apt.data;result={}
    u32=lambda at:struct.unpack_from('<I',data,at)[0]
    u64=lambda at:struct.unpack_from('<Q',data,at)[0]
    align=lambda at:(at+7)&~7
    for _,start in apt.root_streams():
        found={};pool=[];previous=None;p=start
        try:
            while p<len(data):
                op=data[p];p+=1;last=previous;previous=None
                if not op:break
                if op in (0x81,0x87,0x9f,0x99,0x9d,0xb8):p=align(p)+4
                elif op==0x83:p=align(p)+16
                elif op in (0x8b,0x8c,0xa1,0xa4,0xa5,0xa6,0xa7):p=align(p)+8
                elif op in (0x88,0x96):
                    p=align(p);n=u32(p);arr=u64(align(p+4));p=align(p+4)+8
                    if n>20000 or arr+n*8>len(data):raise ValueError('Invalid root constant array')
                    if op==0x88:pool=[apt.const[u64(arr+i*8)] for i in range(n)]
                elif op in (0x8e,0x9b):
                    p=align(p);off=p;n=u32(p+24)
                    if n>len(data)-p-48 or u32(p+8)>64:raise ValueError('Invalid function declaration')
                    if last is not None and last<len(pool) and isinstance(pool[last],str):found[off]=(pool,last,pool[last])
                    p+=48+n
                elif op==0x8f:p=align(align(p)+16)+8
                elif op==0x94:p=align(p)+8
                elif op in (0x77,0xb4,0xb7):p+=4
                elif op in (0xac,0xad):p+=1+data[p]
                elif op in (0xa2,0xa3):
                    previous=data[p] if op==0xa2 else struct.unpack_from('<H',data,p)[0];p+=1 if op==0xa2 else 2
                elif op in (0xa8,0xa9,0xaa,0xab,0xae,0xaf,0xb0,0xb1,0xb2,0xb3,0xb5,0xb9) or 0x78<=op<=0x7f:p+=1
                elif op==0xb6:p+=2
                elif op<=0x76 or op in (0x80,0x82,0x8a,0x9a,0x9e,0xba):pass
                else:raise ValueError(f'Unsupported root opcode {op:02x}')
                if p>len(data):raise ValueError('Root exceeds asset')
            result.update(found)
        except (ValueError,IndexError,struct.error,OverflowError):continue
    return result
