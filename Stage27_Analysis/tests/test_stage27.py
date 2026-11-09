#!/usr/bin/env python3
"""Stdlib-only Stage27 parser, pairing and PE RVA mapping regression tests."""
import pathlib
import struct
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "tools"))
import analyze_bit0_lifecycle as sut

MARKER = "[2026-11-09 17:28:21] STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1"


def line(seq, old, new, rva, eip, eax, esi, edi, ebx, stack):
    return (
        "[2026-11-09 17:28:37] STAGE26_OBJECT_BIT0_WRITE "
        "seq=%d side=P1 tid=18044 tick=100 time=17:28:37.123 qpc=0x%016X "
        "eipAfter=0x%08X inAALib=%d rvaAfter=0x%08X "
        "player=0x00001000 field=0x000010F8 old=0x%08X new=0x%08X "
        "shadowInitialized=1 oldBit0=%d newBit0=%d shadowBit0Changed=1 dr6=0xFFFF0FF0 "
        "EAX=%08X EBX=%08X ECX=00000001 EDX=00000000 ESI=%08X EDI=%08X "
        "EBP=00000000 ESP=00002000 EFLAGS=00000202 bytes_at_EIP_minus_16=00 "
        "stackMask=0xFF stack=[%s]\n"
    ) % (seq, 100000 + 1000 * seq, eip, 1 if rva else 0, rva, old, new, old, new,
         eax, ebx, esi, edi, ",".join("%08X" % n for n in stack))


class Stage27Tests(unittest.TestCase):
    def sample(self):
        source = [0, 0, 0, 0, 0, 0, 0, 0]
        caller = [0x1040, 0x6479D1, 0x1030, 0, 0x64760E, 0, 0, 0]
        return MARKER + "\n" + \
            line(1, 0, 1, 0, sut.SETTER_EIP, 0x1000, 0x2000, 31, 30, source) + \
            line(2, 1, 0, 0x9787A, 0x64787A, 0x1030, 0x1030, 0x1030, 0x1000, caller)

    def test_pairing_and_caller(self):
        rows = sut.parse_log(self.sample())
        self.assertEqual([r["seq"] for r in rows], [1, 2])
        result = sut.analyze(rows)
        self.assertEqual(result["events"], 2)
        self.assertEqual(result["pairs"], 1)
        self.assertEqual(result["sets"], 1)
        self.assertEqual(result["clears"], 1)
        self.assertEqual(result["runtime_game_base"], "0x005B0000")
        self.assertEqual(result["dominant_clear_caller_returns"], {"0x979D1": 1})
        self.assertEqual(result["anomalies"], [])
        self.assertEqual(result["cycles"][0]["duration_qpc_ticks"], 1000)

    def test_missing_marker(self):
        with self.assertRaisesRegex(ValueError, "Missing exact Stage26 marker"):
            sut.parse_log(self.sample().replace(MARKER, ""))

    def test_reject_gap_and_alias_mismatch(self):
        with self.assertRaisesRegex(ValueError, "sequence has gaps"):
            sut.parse_log(self.sample().replace("seq=2", "seq=3"))
        with self.assertRaisesRegex(ValueError, "Field/player alias"):
            sut.parse_log(self.sample().replace("field=0x000010F8", "field=0x000010FC", 1))

    def test_detect_unpaired_clear(self):
        rows = sut.parse_log(self.sample())
        result = sut.analyze(rows[1:])
        self.assertTrue(any("Unpaired clear" in x for x in result["anomalies"]))

    def test_pe_section_rva_mapping(self):
        blob = bytearray(0x500)
        blob[:2] = b"MZ"
        struct.pack_into("<I", blob, 0x3C, 0x80)
        blob[0x80:0x84] = b"PE\x00\x00"
        struct.pack_into("<H", blob, 0x86, 1)
        struct.pack_into("<H", blob, 0x94, 0xE0)
        section = 0x80 + 24 + 0xE0
        struct.pack_into("<IIII", blob, section + 8, 0x100, 0x1000, 0x100, 0x200)
        blob[0x200:0x203] = b"ABC"
        view = sut.pe_rva_mapper(blob)
        self.assertEqual(view(0x1000, 3), b"ABC")
        with self.assertRaisesRegex(ValueError, "not backed"):
            view(0x1100, 1)


if __name__ == "__main__":
    unittest.main()
