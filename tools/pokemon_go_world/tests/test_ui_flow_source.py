import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location("ui_flow_source", Path(__file__).parents[1] / "ui_flow_source.py")
source = importlib.util.module_from_spec(spec)
spec.loader.exec_module(source)


class FlowSourceTests(unittest.TestCase):
    def test_comments_and_literals_keep_lines(self):
        text = '// comment\n"quoted { }"; /* more\n */\n'
        masked = source.mask_comments_and_literals(text)
        self.assertEqual(text.count('\n'), masked.count('\n'))
        self.assertNotIn('{', masked)

    def test_definition_not_prototype_or_comment(self):
        text = 'static void Run(int x);\n// void Fake(void) {}\nstatic void Run(int x)\n{ if(x) { x++; } }\n'
        self.assertEqual(source.extract_function(text, 'Run'), 'static void Run(int x)\n{ if(x) { x++; } }')

    def test_callback_parameter_and_literal_braces(self):
        text = 'void Run(void (*done)(void)) { puts("}"); done(); }'
        self.assertEqual(source.extract_function(text, 'Run'), text)

    def test_missing_and_ambiguous_fail(self):
        for text in ('', 'void Run(void) {}\nvoid Run(void) {}'):
            with self.assertRaises(ValueError): source.extract_function(text, 'Run')

    def test_enum_ignores_comments_references_and_count_is_explicit(self):
        text = 'enum {\n MODE_A, // MODE_FAKE\n MODE_B = 2,\n MODE_COUNT\n};\nif (MODE_GHOST) {}\n'
        self.assertEqual(source.catalog_symbols(text, 'MODE_', 'enum'), {'MODE_A', 'MODE_B', 'MODE_COUNT'})

    def test_defines(self):
        self.assertEqual(source.catalog_symbols('#define ACT_A 1\n// #define ACT_B 2', 'ACT_', 'define'), {'ACT_A'})

    def test_hgss_bool_definition(self):
        text = 'BOOL Main(int *state) { if (*state) { return TRUE; } return FALSE; }'
        self.assertEqual(source.extract_function(text, 'Main'), text)


if __name__ == '__main__': unittest.main()
