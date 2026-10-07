"""Read pinned HGSS cursor animation metadata, without activating game assets.

Offsets follow g2d_Anim_data.h and NNS_G2dUnpackNAN in lib/asm/nnsys.s.
This does not render cells, interpret frame transforms or emulate the DS SDK.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from audit_hgss_ui_reference import REVISION, narc_directory


def lz10(data, limit=16 * 1024 * 1024):
    if len(data) < 4 or data[0] != 0x10:
        raise ValueError('Expected DS LZ10')
    size = int.from_bytes(data[1:4], 'little')
    if not 0 < size <= limit:
        raise ValueError('Invalid decompressed size')
    out, pos = bytearray(), 4
    while len(out) < size:
        if pos >= len(data): raise ValueError('Truncated flags')
        flags, pos = data[pos], pos + 1
        for bit in range(7, -1, -1):
            if len(out) == size: break
            if flags & (1 << bit):
                if pos + 2 > len(data): raise ValueError('Truncated back reference')
                a, b = data[pos:pos + 2]
                pos += 2
                length, distance = (a >> 4) + 3, ((a & 15) << 8 | b) + 1
                if distance > len(out) or len(out) + length > size:
                    raise ValueError('Invalid back reference')
                for _ in range(length): out.append(out[-distance])
            else:
                if pos >= len(data): raise ValueError('Truncated literal')
                out.append(data[pos])
                pos += 1
    return bytes(out)


def narc_member(data, member):
    directory = narc_directory(data)
    if not 0 <= member < len(directory): raise ValueError('Absent NARC member')
    blocks, pos = {}, 16
    for _ in range(struct.unpack_from('<H', data, 14)[0]):
        tag, size = struct.unpack_from('<4sI', data, pos)
        blocks[tag] = data[pos + 8:pos + size]
        pos += size
    start, end = struct.unpack_from('<II', blocks[b'BTAF'], 4 + member * 8)
    return blocks[b'GMIF'][start:end]


def nanr_sequences(data):
    if len(data) < 16 or data[:4] != b'RNAN': raise ValueError('Expected NANR')
    bom, version, size, header, count = struct.unpack_from('<HHIHH', data, 4)
    if (bom, version, size, header) != (0xFEFF, 0x100, len(data), 16):
        raise ValueError('Unsupported NANR header')
    blocks, pos = {}, header
    for _ in range(count):
        if pos + 8 > len(data): raise ValueError('Truncated block')
        tag, length = struct.unpack_from('<4sI', data, pos)
        if length < 8 or pos + length > len(data) or tag in blocks:
            raise ValueError('Invalid block')
        blocks[tag] = data[pos + 8:pos + length]
        pos += length
    if pos != len(data) or b'KNBA' not in blocks: raise ValueError('Missing animation bank')
    bank = blocks[b'KNBA']
    if len(bank) < 24: raise ValueError('Truncated bank')
    seq_count, frame_count, seq_head, frame_head, contents, strings, extended = struct.unpack_from('<HHIIIII', bank)
    if not (24 <= seq_head <= seq_head + seq_count * 16 <= frame_head
            <= frame_head + frame_count * 8 <= contents < len(bank)):
        raise ValueError('Invalid bank sections')
    for extra in (strings, extended):
        if extra and not contents <= extra < len(bank): raise ValueError('Invalid optional section')
    sequences = []
    for i in range(seq_count):
        frames, loop, kind, mode, offset = struct.unpack_from('<HHIII', bank, seq_head + i * 16)
        if not frames or loop >= frames or mode not in (1, 2, 3, 4):
            raise ValueError('Invalid sequence')
        if offset % 8 or offset + frames * 8 > frame_count * 8:
            raise ValueError('Invalid frame array')
        durations, content_offsets = [], []
        for j in range(frames):
            content, ticks, _ = struct.unpack_from('<IHH', bank, frame_head + offset + j * 8)
            if contents + content + 2 > len(bank): raise ValueError('Invalid frame content')
            durations.append(ticks)
            content_offsets.append(content)
        sequences.append({'index': i, 'animation_type': kind, 'play_mode': mode,
            'loop_start_frame': loop, 'durations_ticks': durations,
            'forward_duration_ticks': sum(durations), 'content_offsets': content_offsets})
    return {'sequence_count': seq_count, 'declared_frame_count': frame_count,
        'sequences': sequences, 'scope': 'timing/offset metadata only; cell transforms not decoded'}


def collect(reference):
    reference = reference.resolve()
    # This read-only checkout can belong to the other desktop sandbox account.
    # Limit trust to this invocation/path; do not change global Git configuration.
    git = ['git', '-c', 'safe.directory=' + reference.as_posix(), '-C', str(reference)]
    revision = subprocess.check_output([*git, 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION: raise ValueError('Wrong HGSS revision')
    if subprocess.check_output([*git, 'status', '--porcelain'], text=True).strip():
        raise ValueError('Dirty reference')
    paths = ('src/battle/battle_cursor.c', 'src/battle/battle_input.c',
        'src/battle/overlay_12_0226BEC4.c', 'asm/unk_02077678.s',
        'include/battle/battle_cursor.h', 'include/battle/battle_input.h',
        'lib/include/nnsys/g2d/fmt/g2d_Anim_data.h', 'lib/asm/nnsys.s', 'files/a/0/0/8')
    hashes = {p: hashlib.sha256((reference / p).read_bytes()).hexdigest() for p in paths}
    archive = (reference / 'files/a/0/0/8').read_bytes()
    resources = []
    for member, role in ((80, 'palette'), (250, 'characters'), (251, 'cells'), (252, 'animation')):
        raw = narc_member(archive, member)
        payload = lz10(raw) if raw[:1] == b'\x10' else raw
        resources.append({'member': member, 'role': role, 'stored_sha256': hashlib.sha256(raw).hexdigest(),
            'stored_size': len(raw), 'decoded_size': len(payload), 'decoded_magic': payload[:4].decode('ascii'),
            'decoded_sha256': hashlib.sha256(payload).hexdigest()})
        if role == 'animation': animation = nanr_sequences(payload)
    return {'schema': 1, 'revision': revision, 'source_sha256': hashes,
        'archive': 'files/a/0/0/8', 'cursor_resources': resources, 'animation': animation,
        'status': 'reference only; not ported; not visually validated'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--verify', type=Path)
    args = parser.parse_args()
    report = collect(args.reference)
    if args.output:
        args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    if args.verify and json.loads(args.verify.read_text(encoding='utf-8')) != report:
        raise ValueError('Reference animation differs from evidence')
    print(json.dumps(report['animation']))
