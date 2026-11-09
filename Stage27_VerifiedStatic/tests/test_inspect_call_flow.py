import pathlib
import struct
import sys
import unittest
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "tools"))
import inspect_call_flow as flow


class CallFlowTests(unittest.TestCase):
    def test_rel32_targets(self):
        def view(rva, length):
            self.assertEqual(length, 5)
            target = flow.CALLS[rva]
            return b'\xE8' + struct.pack('<i', target - (rva + 5))
        for call, target in flow.CALLS.items():
            found, _ = flow.direct_rel32_target(view, call)
            self.assertEqual(found, target)

    def test_reject_non_call(self):
        with self.assertRaisesRegex(ValueError, 'Expected E8'):
            flow.direct_rel32_target(lambda _r, _n: b'\x90' * 5, 0x97609)

    def test_pe_rva_mapper(self):
        data = bytearray(0x500)
        data[:2] = b'MZ'
        struct.pack_into('<I', data, 0x3C, 0x80)
        data[0x80:0x84] = b'PE\0\0'
        struct.pack_into('<H', data, 0x84, 0x14C)
        struct.pack_into('<H', data, 0x86, 1)
        struct.pack_into('<H', data, 0x94, 0xE0)
        section = 0x80 + 24 + 0xE0
        struct.pack_into('<IIII', data, section + 8, 0x100, 0x1000, 0x100, 0x200)
        data[0x200:0x204] = b'ABCD'
        view = flow.map_pe(data)
        self.assertEqual(view(0x1000, 4), b'ABCD')
        with self.assertRaisesRegex(ValueError, 'cannot be read'):
            view(0x1100, 1)

    def test_bad_machine(self):
        data = bytearray(0x300)
        data[:2] = b'MZ'
        struct.pack_into('<I', data, 0x3C, 0x80)
        data[0x80:0x84] = b'PE\0\0'
        struct.pack_into('<H', data, 0x84, 0x8664)
        with self.assertRaisesRegex(ValueError, 'Expected x86'):
            flow.map_pe(data)

    def test_windows_nonempty_and_cover_calls(self):
        self.assertGreaterEqual(len(flow.WINDOWS), 3)
        self.assertTrue(any(s <= 0x979CC and s + z > 0x979CC for _, s, z in flow.WINDOWS))
        self.assertTrue(any(s <= 0x97970 and s + z > 0x97970 for _, s, z in flow.WINDOWS))


if __name__ == '__main__':
    unittest.main()
