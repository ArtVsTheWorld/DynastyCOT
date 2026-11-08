"""Read-only PE/binding report. Does not execute, modify or locate new symbols."""
from pathlib import Path
import argparse
import hashlib
import json
import mmap
import re
import struct


class PE:
    def __init__(self, data):
        self.data = data
        if len(data) < 64 or data[:2] != b'MZ':
            raise ValueError('Not a PE executable')
        start = struct.unpack_from('<I', data, 0x3c)[0]
        if start + 24 > len(data) or data[start:start + 4] != b'PE\0\0':
            raise ValueError('Invalid PE header')
        self.machine, count, self.timestamp = struct.unpack_from('<HHI', data, start + 4)
        size = struct.unpack_from('<H', data, start + 20)[0]
        optional = start + 24
        if self.machine != 0x8664 or not 1 <= count <= 96 or size < 112 or optional + size + count * 40 > len(data):
            raise ValueError('Unsupported or truncated x64 PE')
        if struct.unpack_from('<H', data, optional)[0] != 0x20b:
            raise ValueError('Expected PE32+')
        self.base = struct.unpack_from('<Q', data, optional + 24)[0]
        self.image_size, self.headers = struct.unpack_from('<II', data, optional + 56)
        self.sections = []
        for index in range(count):
            pos = optional + size + index * 40
            virtual_size, rva, raw_size, offset = struct.unpack_from('<IIII', data, pos + 8)
            if raw_size and offset + raw_size > len(data):
                raise ValueError('Truncated section data')
            self.sections.append({'name': data[pos:pos + 8].split(b'\0')[0].decode('ascii', errors='replace'),
                                  'rva': rva, 'virtualSize': virtual_size, 'fileSize': raw_size, 'fileOffset': offset})

    def bytes_at(self, rva, count=16):
        if rva < 0 or count < 0:
            return None
        if rva < self.headers:
            return bytes(self.data[rva:min(rva + count, self.headers, len(self.data))])
        for section in self.sections:
            delta = rva - section['rva']
            if 0 <= delta < max(section['virtualSize'], section['fileSize']):
                if delta >= section['fileSize']:
                    return None  # Virtual/zero-filled storage is not present in the file.
                offset = section['fileOffset'] + delta
                return bytes(self.data[offset:offset + min(count, section['fileSize'] - delta)])
        return None


def bindings(text):
    result = {}
    for declaration in re.findall(r'inline constexpr uintptr_t\s+([^;]+);', text):
        for name, value in re.findall(r'(\w+)\s*=\s*(0x[0-9a-fA-F]+)', declaration):
            if int(value, 16) >= 0x1000:  # Exclude small structure offsets such as CrowdSlider.
                result[name] = int(value, 16)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--bindings', type=Path, default=Path(__file__).resolve().parents[1] / 'Source/GameBindings.h')
    args = parser.parse_args()
    if args.exe.resolve() == args.out.resolve():
        parser.error('The output must not replace the executable')
    names = bindings(args.bindings.read_text(encoding='utf-8-sig'))
    try:
        with args.exe.open('rb') as file:
            with mmap.mmap(file.fileno(), 0, access=mmap.ACCESS_READ) as data:
                image = PE(data)
                entries = []
                for name, rva in names.items():
                    found = image.bytes_at(rva)
                    entries.append({'name': name, 'rva': hex(rva), 'fileBytes': found.hex(' ') if found is not None else None})
                report = {'file': args.exe.name, 'sha256': hashlib.sha256(data).hexdigest(),
                          'imageBase': hex(image.base), 'imageSize': image.image_size, 'peTimestamp': image.timestamp,
                          'sections': image.sections, 'existingBindings': entries,
                          'limitation': 'Bytes at existing RVAs only. Does not discover new addresses, validate layouts, patch the file or certify compatibility. SHA256 identifies this report; it is not a plugin requirement.'}
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print('Recorded ' + str(len(names)) + ' named addresses in ' + str(args.out))
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, 'Inspection failed: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
