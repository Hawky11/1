#!/usr/bin/env python3
"""Small synthetic tests for conservative Stage64 argument inference."""
import unittest
from analyze_stage64 import conservative_args


class TestStage64(unittest.TestCase):
    def test_literal_id_mode_zero(self):
        rows = [
            (0x4000, 'push   0x0', b'\x6a\x00'),
            (0x4002, 'push   0x15', b'\x6a\x15'),
            (0x4004, 'mov    ecx,esi', b'\x8b\xce'),
            (0x4006, 'call   0x4e5bf0', b'\xe8\x00\x00\x00\x00'),
        ]
        result = conservative_args(rows, 3)
        self.assertEqual(result['mode'], '0')
        self.assertEqual(result['immediate_id'], 0x15)

    def test_register_mode_one(self):
        rows = [
            (0x4000, 'push   0x1', b'\x6a\x01'),
            (0x4002, 'push   eax', b'\x50'),
            (0x4003, 'mov    ecx,edi', b'\x8b\xcf'),
            (0x4005, 'call   0x4e5bf0', b'\xe8\x00\x00\x00\x00'),
        ]
        result = conservative_args(rows, 3)
        self.assertEqual(result['mode'], '1')
        self.assertEqual(result['id_kind'], 'register')

    def test_branch_breaks_confidence(self):
        rows = [
            (0x4000, 'push   0x1', b'\x6a\x01'),
            (0x4002, 'jne    0x4010', b'\x75\x0c'),
            (0x4004, 'push   eax', b'\x50'),
            (0x4005, 'call   0x4e5bf0', b'\xe8\x00\x00\x00\x00'),
        ]
        self.assertEqual(conservative_args(rows, 3)['mode'], 'unresolved')

    def test_missing_mode(self):
        rows = [
            (0x4000, 'push   0x29', b'\x6a\x29'),
            (0x4002, 'call   0x4e5bf0', b'\xe8\x00\x00\x00\x00'),
        ]
        self.assertEqual(conservative_args(rows, 1)['mode'], 'unresolved')


if __name__ == '__main__':
    unittest.main()
