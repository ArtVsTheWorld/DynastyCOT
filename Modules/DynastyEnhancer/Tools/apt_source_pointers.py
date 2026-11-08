"""Decode exact APT loader pointer operands; unknown instructions fail closed.

The complete source callback stays hashed. Only typed pointer operands may
normalize to their exact source RVA, with or without the asset base.
"""
import struct

def source_pointers(apt,offset,size):
 data=apt.data;start=offset+48;end=start+size;result=[]
 align=lambda n:(n+7)&~7
 def pointer(at):
  value=struct.unpack_from('<Q',data,at)[0]
  if not start<=at<=end-8 or value>=len(data):raise ValueError('Invalid source pointer operand')
  result.append((at-start,value))
 p=start
 while p<end:
  at=p;op=data[p];p+=1
  if op in (0x81,0x87,0x9f,0x99,0x9d,0xb8):p=align(p)+4
  elif op==0x83:
   p=align(p);pointer(p);pointer(p+8);p+=16
  elif op in (0x8b,0x8c,0xa1,0xa4,0xa5,0xa6,0xa7):
   p=align(p);pointer(p);p+=8
  elif op in (0x88,0x96):
   p=align(p);n=struct.unpack_from('<I',data,p)[0]
   if n>=20000:raise ValueError('Invalid constant array')
   p=align(p+4);pointer(p);p+=8
  elif op in (0x8e,0x9b):raise ValueError('Nested definition requires explicit metadata')
  elif op==0x8f:p=align(align(p)+16)+8
  elif op==0x94:p=align(p)+8
  elif op in (0x77,0xb4,0xb7):p+=4
  elif op in (0xac,0xad):
   # EA compact dictionary chains: byte count, then byte indices.
   n=data[p];p+=1+n
  elif op in (0xa2,0xa8,0xa9,0xaa,0xab,0xae,0xaf,0xb0,0xb1,0xb2,0xb3,0xb5,0xb9) or 0x78<=op<=0x7f:p+=1
  elif op in (0xa3,0xb6):p+=2
  elif op<=0x76 or op in (0x80,0x82,0x8a,0x9a,0x9e,0xba):pass
  else:raise ValueError(f'Unsupported APT opcode {op:02x} at {at:x}')
  if p>end:raise ValueError('Instruction exceeds source callback')
 if p!=end:raise ValueError('Source callback extent differs')
 return sorted(set(result))
