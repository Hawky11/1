# Stage62 — offline action-chain analysis

Runtime-verified same-launch results: [RUNTIME_ANALYSIS.md](../Stage62_Runtime/RUNTIME_ANALYSIS.md).

The Python 3 source [correlate_stage62.py](tools/correlate_stage62.py) is **read-only**, uses only the standard library, and does not attach to the game, write memory or load the DLL. It checks exact session stamps, P1/P2 actors, native game thread, QPC order, original action IDs, observer sequence IDs, definition binding and the Stage43 original caller. The 44 subset chains also require Stage37 type1 payload equality and Stage39 P+3F4C output.

Usage in Windows PowerShell:

    py -3 .\Stage62_Analysis\tools\correlate_stage62.py --dir D:\INCUBATOR\ArcanaAIOverhaul --json .\Stage62_analysis.json

Place **the original eight 22:07 session logs** together in the --dir folder: Stage37Type1CommitTrace, Stage39ActionOutputTrace, Stage40ActionConsumerTrace, Stage41ObjectCommitTrace, Stage42ActionActivationTrace, Stage43ActivationOriginTrace, Stage44ActionDefinitionTrace, and Stage46DefinitionMotionTrace (all prefixed ArcanaAI_ and suffixed .log). The program prints summary counts and optionally writes a local JSON report; that JSON is not committed.

For the fixed uploaded snapshot, independent analysis of the actual eight logs established 48/44/263/263/202/202/1476/1476 rows, **202 strict activation links**, **44 strict upstream callback links**, **61 Stage40 duplicate requests all ID 0x49**, and 4 Stage37 effects not matched to an action-output observer. The new Python reproduction script has **not yet been executed on the user's Windows machine**; do not misrepresent this as a fresh Windows test.

DO NOT install another DLL, change INI, force an action ID, or infer smart AI/animation/attack improvement from these correlations. The next investigation is the native **source of the P+3F4C write / AI choice**, not more coordinate watchpoints.
