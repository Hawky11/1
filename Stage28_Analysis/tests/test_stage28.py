#!/usr/bin/env python3
"""Stage28 signature/PE reader regression tests (offline, Python stdlib)."""
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
import verify_stage28 as s

class Stage28Tests(unittest.TestCase):
    def test_markers(self):
        self.assertIn(0x978B0,s.SIGNATURES)
        self.assertIn(0x97830,s.SIGNATURES)
        self.assertEqual(s.DIRECT_CALLS[0x978F5],0xF2ED0)
        self.assertEqual(s.DIRECT_CALLS[0x97958],0x97830)
        self.assertEqual(s.DIRECT_JUMPS[0x979AC],0x97870)

    def test_rel32_call(self):
        a=0x978F5
        read=lambda _a,_n: b"\xE8"+struct.pack("<i",0xF2ED0-a-5)
        self.assertEqual(s.rel32_target(read,a,0xE8),0xF2ED0)
        with self.assertRaisesRegex(ValueError,"expected opcode"):
            s.rel32_target(read,a,0xE9)

    def test_rel32_tail_jmp(self):
        a=0x979AC
        read=lambda _a,_n: b"\xE9"+struct.pack("<i",0x97870-a-5)
        self.assertEqual(s.rel32_target(read,a,0xE9),0x97870)

    def test_pe_rva_mapping(self):
        d=bytearray(0x500)
        d[:2]=b"MZ"
        struct.pack_into("<I",d,0x3C,0x80)
        d[0x80:0x84]=b"PE\x00\x00"
        struct.pack_into("<H",d,0x84,0x14C)
        struct.pack_into("<H",d,0x86,1)
        struct.pack_into("<H",d,0x94,0xE0)
        sec=0x80+24+0xE0
        struct.pack_into("<IIII",d,sec+8,0x100,0x1000,0x100,0x200)
        d[0x200:0x205]=b"ABCDE"
        read=s.pe_rva_reader(d)
        self.assertEqual(read(0x1000,5),b"ABCDE")
        with self.assertRaisesRegex(ValueError,"Unmapped"):
            read(0x1100,1)

    def test_reject_x64(self):
        d=bytearray(0x300)
        d[:2]=b"MZ"
        struct.pack_into("<I",d,0x3C,0x80)
        d[0x80:0x84]=b"PE\x00\x00"
        struct.pack_into("<H",d,0x84,0x8664)
        with self.assertRaisesRegex(ValueError,"Expected x86"):
            s.pe_rva_reader(d)

    def test_real_binary_if_provided(self):
        import os
        raw=os.environ.get("STAGE28_AALIB_EXE")
        if not raw:
            self.skipTest("Set STAGE28_AALIB_EXE to run exact original game PE test")
        result=s.inspect(Path(raw))
        self.assertEqual(result["verified_sha256"],s.SHA256)
        self.assertEqual(result["signatures_checked"],26)
        self.assertEqual(result["direct_calls_checked"],16)

if __name__=="__main__":
    unittest.main()
