"""Execute exact pinned HGSS button feedback functions with simulated graphics.

Reference investigation only: this is not a port or visual acceptance test.
"""
import argparse
from pathlib import Path
import subprocess
from audit_hgss_animation import collect
from ui_flow_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def main(reference):
    collect(reference)  # Verify revision, clean checkout and resource metadata first.
    source = (reference / 'src/battle/battle_input.c').read_text(encoding='utf-8')
    functions = [extract_function(source, name) for name in
                 ('ov12_022698B0', 'BattleInput_CheckFeedbackDone',
                  'BattleInput_GetKeyPressed', 'BattleInput_SetKeyPressed',
                  'Task_ButtonFeedback', 'Task_FightMenuButtonFeedback')]
    fixture = (ROOT / 'tools/pokemon_go_world/tests/hgss_button_feedback_native.c').read_text(encoding='utf-8')
    if fixture.count('/* PRODUCTION_FUNCTIONS */') != 1: raise ValueError('Invalid fixture marker')
    build = ROOT / 'build/hgss-feedback-reference'
    build.mkdir(parents=True, exist_ok=True)
    generated = build / 'fixture.c'
    generated.write_text(fixture.replace('/* PRODUCTION_FUNCTIONS */', '\n'.join(functions)), encoding='utf-8')
    compiler = ROOT.parent / 'toolchains/winlibs-i686-r4-tar/mingw32/bin/gcc.exe'
    executable = build / 'fixture.exe'
    subprocess.run([str(compiler), '-std=gnu17', '-iquote', str(reference / 'include'),
                    str(generated), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', required=True, type=Path)
    main(parser.parse_args().reference.resolve())
