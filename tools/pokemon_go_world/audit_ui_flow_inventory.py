"""Audit explicitly classified UI catalogs and index source references/states.

All preprocessor branches are indexed. A symbol reference is NOT proof that a
branch runs on PC. Reachability/configuration decisions live in the F0.2 matrix.
New unclassified catalog values fail the audit instead of silently disappearing.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
from ui_flow_source import catalog_symbols, function_blocks, mask_comments_and_literals

ROOT = Path(__file__).resolve().parents[2]
SCOPE = ROOT / 'docs/pokemon_regionalidades/HGSS_UI_FLOW_SCOPE.json'


def validate_catalog(actual, groups, prefix, flows):
    declared = {}
    for flow, suffixes in groups.items():
        if flow not in flows: raise ValueError(f'Unknown flow: {flow}')
        for suffix in suffixes.split():
            symbol = prefix + suffix
            if symbol in declared: raise ValueError(f'Duplicate classification: {symbol}')
            declared[symbol] = flow
    if actual != set(declared):
        raise ValueError(f'Catalog changed: unclassified={sorted(actual - set(declared))}; removed={sorted(set(declared) - actual)}')
    return declared


def collect(root=ROOT, scope_path=SCOPE):
    scope = json.loads(scope_path.read_text(encoding='utf-8'))
    catalogs, references, states, hashes = [], {}, {}, {}
    entry_flows = {}
    for flow, spec in scope['flows'].items():
        if not spec['phase'] or not spec['result']: raise ValueError(f'Missing contract: {flow}')
        for symbol in spec['entrypoints'].split():
            entry_flows.setdefault(symbol, []).append(flow)
            references[symbol] = []
    for catalog in scope['catalogs']:
        text = (root / catalog['file']).read_text(encoding='utf-8')
        actual = catalog_symbols(text, catalog['prefix'], catalog['kind'])
        classified = validate_catalog(actual, catalog['groups'], catalog['prefix'], scope['flows'])
        catalogs.append({'file': catalog['file'], 'symbols': classified})
    pattern = re.compile(r'\b(' + '|'.join(map(re.escape, entry_flows)) + r')\b')
    paths = sorted((root / 'src').rglob('*.c')) + sorted((root / 'data').rglob('*.inc'))
    for path in paths:
        raw = path.read_bytes()
        text = raw.decode('utf-8', errors='replace')
        relative = path.relative_to(root).as_posix()
        masked = mask_comments_and_literals(text)
        hits = list(pattern.finditer(masked))
        if hits or relative in scope['state_sources']:
            hashes[relative] = hashlib.sha256(raw).hexdigest()
        for match in hits:
            references[match.group()].append({'file': relative, 'line': text.count('\n', 0, match.start()) + 1})
        if relative in scope['state_sources']:
            states[relative] = []
            for block in function_blocks(text):
                body = mask_comments_and_literals(block['body'])
                calls = re.findall(r'\b(?:SetMainCallback2|SetTaskFuncWithFollowupFunc|CreateTask)\s*\(([^;]*?)\)\s*;', body)
                assignments = re.findall(r'(?:\.func|Callback|ControllerFuncs\[[^\]]+\])\s*=\s*([^;\n]+)', body)
                if calls or assignments or block['name'].startswith(('Task_', 'CB2_')):
                    states[relative].append({'function': block['name'], 'line': block['line'],
                        'callback_calls': [' '.join(c.split()) for c in calls],
                        'callback_assignments': [' '.join(a.split()) for a in assignments]})
    missing = sorted(symbol for symbol, locations in references.items() if not locations)
    if missing: raise ValueError(f'Unlocated entrypoints: {missing}')
    missing_sources = set(scope['state_sources']) - set(states)
    if missing_sources: raise ValueError(f'Missing state sources: {sorted(missing_sources)}')
    for catalog in scope['catalogs']:
        hashes[catalog['file']] = hashlib.sha256((root / catalog['file']).read_bytes()).hexdigest()
    return {'schema': 1, 'method': 'lexical source inventory; includes inactive branches; not a runtime graph',
            'scope_sha256': hashlib.sha256(scope_path.read_bytes()).hexdigest(), 'source_sha256': hashes,
            'catalogs': catalogs, 'entry_flows': entry_flows, 'references': references, 'state_sites': states}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--verify', type=Path)
    args = parser.parse_args()
    report = collect()
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    if args.verify and json.loads(args.verify.read_text(encoding='utf-8')) != report:
        raise ValueError('Inventory differs: review sources/classification and regenerate explicitly')
    print(json.dumps({'catalog_values': sum(len(c['symbols']) for c in report['catalogs']),
        'entrypoints': len(report['references']), 'source_references': sum(map(len, report['references'].values())),
        'state_sites': sum(map(len, report['state_sites'].values())), 'source_files': len(report['source_sha256'])}))
