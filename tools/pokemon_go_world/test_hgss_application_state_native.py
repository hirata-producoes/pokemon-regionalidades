"""Test pinned original cursor/panel functions with memory-only collaborators."""
import argparse
from pathlib import Path
import subprocess
from audit_hgss_applications import collect
from ui_flow_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def main(reference):
    collect(reference)
    bag = (reference / 'src/bag.c').read_text(encoding='utf-8')
    names = ('BagCursor_Field_PocketGetPosition', 'BagCursor_Field_GetPocket',
        'BagCursor_Field_PocketSetPosition', 'BagCursor_Field_SetPocket',
        'BagCursor_Battle_PocketGetPosition', 'BagCursor_Battle_GetLastUsedItem',
        'BagCursor_Battle_GetLastUsedPocket', 'BagCursor_Battle_GetPocket',
        'BagCursor_Battle_PocketSetPosition', 'BagCursor_Battle_Init',
        'BagCursor_Battle_SetLastUsedItem', 'BagCursor_Battle_SetPocket')
    definitions = [extract_function(bag, name) for name in names]
    party = (reference / 'src/party_menu.c').read_text(encoding='utf-8')
    definitions.append(extract_function(party, 'PartyMenu_UpdateTopScreenPanelYCoordFrame'))
    fixture = (ROOT / 'tools/pokemon_go_world/tests/hgss_application_state_native.c').read_text(encoding='utf-8')
    if fixture.count('/* PRODUCTION_FUNCTIONS */') != 1: raise ValueError('Invalid marker')
    build = ROOT / 'build/hgss-application-reference'
    build.mkdir(parents=True, exist_ok=True)
    generated = build / 'fixture.c'
    generated.write_text(fixture.replace('/* PRODUCTION_FUNCTIONS */', '\n'.join(definitions)), encoding='utf-8')
    executable = build / 'fixture.exe'
    compiler = ROOT.parent / 'toolchains/winlibs-i686-r4-tar/mingw32/bin/gcc.exe'
    subprocess.run([str(compiler), '-std=gnu17', '-iquote', str(reference / 'include'),
        str(generated), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    main(parser.parse_args().reference.resolve())
