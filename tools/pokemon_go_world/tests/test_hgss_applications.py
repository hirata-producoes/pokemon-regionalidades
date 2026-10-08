import importlib.util
from pathlib import Path
import sys
import unittest

TOOLS = Path(__file__).parents[1]
sys.path.insert(0, str(TOOLS))
spec = importlib.util.spec_from_file_location('hgss_applications', TOOLS / 'audit_hgss_applications.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)

SOURCE = '''
thumb_func_start Main
Main:
 .short _A - _T - 2 ; case 0
 .short _B - _T - 2 ; case 1
_A:
 bl Wait
 bne _B
 bl Ready
_B:
 bl Exit
thumb_func_end Main
thumb_func_start Wait
Wait:
 mov r0, #0
thumb_func_end Wait
'''


class ApplicationReferenceTests(unittest.TestCase):
    def test_preserves_conditional_block(self):
        states = audit.dispatch(SOURCE, 'Main', 2)
        self.assertEqual(states[0]['direct_calls'], ['Wait', 'Ready'])
        self.assertIn('bne _B', states[0]['block'])
        self.assertIn('Wait', states[0]['handler_locations'])
        self.assertEqual(states[1]['direct_calls'], ['Exit'])

    def test_missing_state_rejected(self):
        with self.assertRaises(ValueError): audit.dispatch(SOURCE, 'Main', 3)

    def test_reordered_state_rejected(self):
        with self.assertRaises(ValueError): audit.dispatch(SOURCE.replace('case 1', 'case 0'), 'Main', 2)

    def test_missing_label_rejected(self):
        with self.assertRaises(ValueError): audit.dispatch(SOURCE.replace('_B:', '_C:'), 'Main', 2)

    def test_truncated_function_rejected(self):
        with self.assertRaises(ValueError): audit.assembly_functions(SOURCE.replace('thumb_func_end Wait', ''))

    def test_explicit_table_excludes_nested_dispatch(self):
        text = SOURCE.replace('Main:\n', 'Main:\n_T:\n').replace('_B:\n', '_B:\n_Nested:\n .short _B - _Nested - 2 ; case 0\n')
        self.assertEqual(len(audit.dispatch(text, 'Main', 2, table_label='_T')), 2)
        with self.assertRaises(ValueError): audit.dispatch(text, 'Main', 2)
        with self.assertRaises(ValueError): audit.dispatch(text, 'Main', 2, table_label='_Missing')


if __name__ == '__main__': unittest.main()
