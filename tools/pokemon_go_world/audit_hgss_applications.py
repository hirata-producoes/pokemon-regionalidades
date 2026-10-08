"""Index pinned HGSS bag/summary dispatches and party resources offline.

Assembly calls are references, not an emulated control-flow graph. Numeric
states retain original identities; this tool never invents semantic names.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

from audit_hgss_animation import lz10, nanr_sequences
from audit_hgss_ui_reference import REVISION, narc_directory


def assembly_functions(text):
    pattern = re.compile(r'(?m)^[ \t]*(?:thumb|arm)_func_start (\w+)[ \t]*$')
    functions = {}
    for match in pattern.finditer(text):
        name = match.group(1)
        end = re.search(r'(?m)^[ \t]*(?:thumb|arm)_func_end ' + re.escape(name) + r'[ \t]*$', text[match.end():])
        if end is None or name in functions: raise ValueError('Invalid Assembly function: ' + name)
        body = text[match.end():match.end() + end.start()]
        functions[name] = {'line': text.count('\n', 0, match.start()) + 1,
            'body': body, 'calls': re.findall(r'(?m)^\s*bl\s+(\w+)\s*$', body)}
    return functions


def dispatch(text, name, expected_count, table_label=None):
    functions = assembly_functions(text)
    body = functions[name]['body']
    case_region = body
    if table_label is not None:
        tables = re.findall(r'(?m)^' + re.escape(table_label) + r':[^\n]*\n((?:[ \t]*\.short[^\n]*\n)+)', body)
        if len(tables) != 1: raise ValueError('Missing or duplicate dispatch table: ' + table_label)
        case_region = tables[0]
    cases = re.findall(r'(?m)^\s*\.short\s+(\w+)\s+-[^;]+;\s*case\s+(\d+)\s*$', case_region)
    if [int(n) for _, n in cases] != list(range(expected_count)):
        raise ValueError('Changed or incomplete dispatch: ' + name)
    labels = list(re.finditer(r'(?m)^(\w+):[^\n]*$', body))
    blocks = {m.group(1): body[m.end():labels[i + 1].start() if i + 1 < len(labels) else len(body)]
              for i, m in enumerate(labels)}
    result = []
    for label, number in cases:
        if label not in blocks: raise ValueError('Missing case label: ' + label)
        block = blocks[label]
        calls = re.findall(r'(?m)^\s*bl\s+(\w+)\s*$', block)
        result.append({'state': int(number), 'label': label, 'direct_calls': calls,
            'block': block.strip(), 'handler_locations':
            {call: functions[call]['line'] for call in calls if call in functions}})
    return result


def metadata(raw):
    data = lz10(raw) if raw[:1] == b'\x10' else raw
    result = {'stored_size': len(raw), 'sha256': hashlib.sha256(raw).hexdigest(),
              'decoded_size': len(data), 'magic_hex': data[:4].hex()}
    if data[:4] == b'RNAN': result['animation'] = nanr_sequences(data)
    return result


def collect(reference):
    reference = reference.resolve()
    git = ['git', '-c', 'safe.directory=' + reference.as_posix(), '-C', str(reference)]
    if subprocess.check_output([*git, 'rev-parse', 'HEAD'], text=True).strip() != REVISION:
        raise ValueError('Wrong reference revision')
    if subprocess.check_output([*git, 'status', '--porcelain'], text=True).strip():
        raise ValueError('Dirty reference')
    sources = ('asm/overlay_15.s', 'asm/unk_02088288.s', 'src/bag.c', 'src/bag_view.c',
        'include/bag_types_def.h', 'include/bag_cursor.h', 'include/unk_02088288.h', 'include/filesystem_files_def.h',
        'src/party_menu.c', 'src/party_menu_sprites.c', 'include/party_menu.h', 'src/launch_application.c')
    hashes = {p: hashlib.sha256((reference / p).read_bytes()).hexdigest() for p in sources}
    dispatches = {
        'bag': dispatch((reference / sources[0]).read_text(encoding='utf-8'), 'Bag_Main', 38),
        'summary': dispatch((reference / sources[1]).read_text(encoding='utf-8'), 'PokemonSummary_Main', 23),
    }
    archives = []
    for path in ('files/a/0/1/5', 'files/a/0/3/9', 'files/a/1/6/2', 'files/a/1/8/0'):
        raw = (reference / path).read_bytes()
        hashes[path] = hashlib.sha256(raw).hexdigest()
        archives.append({'path': path, 'members': narc_directory(raw)})
    party_resources = []
    for path in sorted((reference / 'files/graphic/plist_gra').iterdir()):
        if path.is_file():
            relative = path.relative_to(reference).as_posix()
            raw = path.read_bytes()
            hashes[relative] = hashlib.sha256(raw).hexdigest()
            party_resources.append({'path': relative, **metadata(raw)})
    return {'schema': 1, 'revision': REVISION, 'status': 'reference inventory; not ported or rendered',
        'method': 'direct Assembly dispatch blocks; conditional branches preserved as text',
        'source_sha256': hashes, 'dispatches': dispatches, 'archives': archives,
        'party_resources': party_resources}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--verify', type=Path)
    args = parser.parse_args()
    result = collect(args.reference)
    if args.output:
        args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    if args.verify and json.loads(args.verify.read_text(encoding='utf-8')) != result:
        raise ValueError('Application reference differs from evidence')
    print(json.dumps({'dispatches': {k: len(v) for k, v in result['dispatches'].items()},
        'party_resources': len(result['party_resources']),
        'archives': {a['path']: len(a['members']) for a in result['archives']}}))
