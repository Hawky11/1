#!/usr/bin/env python3
"""Offline tests for the Stage61 same-run join. No game, DLL or EXE needed."""
import tempfile
import unittest
from pathlib import Path

from correlate_stage61 import correlate

SOURCE46 = (
    "[2026-11-10 22:07:56] STAGE46_MOTION_WINDOW "
    "stage44Seq=1 side=P1 player=0E4A1F78 tid=15132 "
    "object=2A89A020 bindingQpcBefore=100 resolvedQpc=105 qpcAfter=110 "
    "resolverCount=1 definition=2981F020 definitionAfter=2981F020 "
    "meta30=00000003 f0=00000020>00000020\n"
)
SOURCE47 = (
    "[2026-11-10 22:07:56] STAGE47_CONSUMER_WINDOW "
    "seq=1 side=P1 player=0E4A1F78 object=2A89A020 tid=15132 "
    "callRVA=000AE70C qpcBefore=200 qpcAfter=350 "
    "def=2981F020>2981F020 f0=00000020>00000020 "
    "f5=441E8000>441F8000\n"
)
SOURCE59 = (
    "[2026-11-10 22:07:56] STAGE59_X_WRITE "
    "seq=1 side=P1 tid=15132 watched=2A89A09C qpcAfter=300 "
    "EIP_after=009E70DE RVA_after=001E70DE "
    "oldShadow=441E8000 newRaw=441F8000\n"
)


class Stage61JoinTests(unittest.TestCase):
    def analyze(self, s46=SOURCE46, s47=SOURCE47, s59=SOURCE59):
        with tempfile.TemporaryDirectory() as directory:
            paths = [Path(directory) / f"s{n}.log" for n in (46, 47, 59)]
            for path, contents in zip(paths, (s46, s47, s59)):
                path.write_text(contents, encoding="utf-8")
            return correlate(*paths)

    def test_exact_single_actor_join(self):
        r = self.analyze()
        c = r["counts"]
        self.assertEqual(c["triple_46_47_59"], 1)
        self.assertEqual(c["triple_active_id_agrees"], 1)
        self.assertEqual(c["last_59_matches_47_post_x"], 1)
        self.assertEqual(r["triple_definition_modes"], {"3": 1})

    def test_disjoint_sessions_rejected(self):
        with self.assertRaisesRegex(ValueError, "DISJOINT_QPC"):
            self.analyze(s59=SOURCE59.replace("qpcAfter=300", "qpcAfter=5000"))

    def test_different_actor_rejected(self):
        with self.assertRaisesRegex(ValueError, "NO_COMPARABLE_ACTOR"):
            self.analyze(s46=SOURCE46.replace("2A89A020", "2A35A020"))

    def test_definition_mismatch_not_treated_as_triple(self):
        r = self.analyze(s47=SOURCE47.replace("2981F020", "2999F020"))
        self.assertEqual(r["counts"].get("triple_46_47_59", 0), 0)
        self.assertEqual(r["counts"]["prior_46_definition_mismatch"], 1)


if __name__ == "__main__":
    unittest.main()
