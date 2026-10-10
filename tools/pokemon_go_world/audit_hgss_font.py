"""Read original font headers, advances and glyph hashes for PT-BR coverage.

FontData_Init/GetGlyphWidth define the header and one-based character IDs.
Bitmap glyphs remain packed; this is not a font renderer or a text edit.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
from audit_hgss_ui_reference import REVISION


def font_header(data):
    if len(data) < 16: raise ValueError('Truncated font header')
    header, widths, count, fixed_w, fixed_h, tiles_w, tiles_h = struct.unpack_from('<III4B', data)
    if header < 16 or not count or tiles_w not in (1, 2) or tiles_h not in (1, 2):
        raise ValueError('Invalid font dimensions')
    glyph_size = 16 * tiles_w * tiles_h
    if not (header + count * glyph_size <= widths <= widths + count <= len(data)):
        raise ValueError('Invalid font bounds')
    if fixed_w > tiles_w * 8 or fixed_h > tiles_h * 8:
        raise ValueError('Invalid fixed metrics')
    return {'header_size': header, 'width_data_start': widths, 'glyph_count': count,
        'fixed_width': fixed_w, 'fixed_height': fixed_h, 'tiles_width': tiles_w,
        'tiles_height': tiles_h, 'packed_glyph_size': glyph_size}


def glyph_metadata(data, header, glyph_id):
    if not 1 <= glyph_id <= header['glyph_count']: raise ValueError('Invalid character ID')
    index = glyph_id - 1
    start = header['header_size'] + index * header['packed_glyph_size']
    return {'character_id_hex': f'{glyph_id:04X}', 'zero_based_glyph': index,
        'advance': data[header['width_data_start'] + index],
        'packed_sha256': hashlib.sha256(data[start:start + header['packed_glyph_size']]).hexdigest()}


def collect(reference):
    reference = reference.resolve()
    git = ['git', '-c', 'safe.directory=' + reference.as_posix(), '-C', str(reference)]
    if subprocess.check_output([*git, 'rev-parse', 'HEAD'], text=True).strip() != REVISION:
        raise ValueError('Wrong reference revision')
    if subprocess.check_output([*git, 'status', '--porcelain'], text=True).strip():
        raise ValueError('Dirty reference')
    charmap = (reference / 'charmap.txt').read_text(encoding='utf-8')
    mappings = {}
    for code, char in re.findall(r'(?m)^([0-9A-F]{4})=([^\r\n])$', charmap):
        mappings.setdefault(char, []).append(int(code, 16))
    required = ''.join(dict.fromkeys('ESTAÇÃO MANHÃ OPÇÕES POKÉMON região início Iil AÀÁÂÃÄ CÇ EÈÉ OÕõ'))
    required = required.replace(' ', '')
    for char in required:
        if len(mappings.get(char, [])) != 1: raise ValueError('Missing/ambiguous charmap character: ' + char)
    sources = ('src/font.c', 'src/font_data.c', 'include/font_data.h', 'charmap.txt')
    hashes = {p: hashlib.sha256((reference / p).read_bytes()).hexdigest() for p in sources}
    fonts = []
    for logical, member in enumerate((0, 1, 2, 3, 4, 10)):
        path = f'files/graphic/font/font_{member:08d}.bin'
        data = (reference / path).read_bytes()
        hashes[path] = hashlib.sha256(data).hexdigest()
        header = font_header(data)
        fonts.append({'logical_font': logical, 'member': member, 'path': path, **header,
            'characters': {c: glyph_metadata(data, header, mappings[c][0]) for c in required}})
    return {'schema': 1, 'revision': REVISION, 'source_sha256': hashes, 'fonts': fonts,
        'status': 'original packed font metrics; not converted or visually validated',
        'mapping': 'IDs from charmap, glyph index = ID - 1; variable advances per font'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--verify', type=Path)
    args = parser.parse_args()
    report = collect(args.reference)
    if args.output: args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    if args.verify and json.loads(args.verify.read_text(encoding='utf-8')) != report:
        raise ValueError('Font reference differs from evidence')
    print(json.dumps({'fonts': len(report['fonts']), 'glyphs_per_font': report['fonts'][0]['glyph_count'],
        'characters_checked': len(report['fonts'][0]['characters'])}))
