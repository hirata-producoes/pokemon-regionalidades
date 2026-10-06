import importlib.util
from pathlib import Path
import sys
import unittest

TOOLS = Path(__file__).parents[1]
sys.path.insert(0, str(TOOLS))
spec = importlib.util.spec_from_file_location('flow_inventory', TOOLS / 'audit_ui_flow_inventory.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class FlowInventoryTests(unittest.TestCase):
    def test_complete(self):
        self.assertEqual(audit.validate_catalog({'MODE_A'}, {'flow': 'A'}, 'MODE_', {'flow': {}}), {'MODE_A': 'flow'})

    def test_new_value_blocks_gate(self):
        with self.assertRaises(ValueError): audit.validate_catalog({'MODE_A', 'MODE_B'}, {'flow': 'A'}, 'MODE_', {'flow': {}})

    def test_removed_value_blocks_gate(self):
        with self.assertRaises(ValueError): audit.validate_catalog(set(), {'flow': 'A'}, 'MODE_', {'flow': {}})

    def test_duplicate_blocks_gate(self):
        with self.assertRaises(ValueError): audit.validate_catalog({'MODE_A'}, {'flow': 'A A'}, 'MODE_', {'flow': {}})

    def test_unknown_flow_blocks_gate(self):
        with self.assertRaises(ValueError): audit.validate_catalog({'MODE_A'}, {'missing': 'A'}, 'MODE_', {})

    def test_current_source_catalogs(self):
        report = audit.collect()
        self.assertTrue(report['catalogs'])
        self.assertTrue(report['state_sites']['src/item_menu.c'])


if __name__ == '__main__': unittest.main()
