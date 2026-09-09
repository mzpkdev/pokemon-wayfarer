import unittest

from verify_stack import audit


class StackControlFlowTests(unittest.TestCase):
    def test_branch_loop_and_nested_call_depth(self):
        result = audit("""
100: push {r4, lr}
102: sub sp, #12
104: cmp r0, #0
106: beq.n 10c <example+0xc>
108: bl 200 <child>
10a: b.n 104 <example+0x4>
10c: add sp, #12
10e: pop {r4, pc}
110: .word 0x12345678
""")
        self.assertEqual(result["ownPeakBytes"], 20)
        self.assertEqual(result["calls"], [{"pc": "0x108", "depth": 20, "target": "child"}])

    def test_unbalanced_loop_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "inconsistent stack depth"):
            audit("100: push {lr}\n102: b.n 100 <example>")

    def test_dynamic_and_conditional_stack_writes_are_rejected(self):
        for instruction in ("mov sp, r4", "sub sp, r4", "pushne {lr}"):
            with self.subTest(instruction=instruction), self.assertRaises(ValueError):
                audit("100: " + instruction + "\n102: bx lr")

    def test_live_tail_call_and_data_fallthrough_are_rejected(self):
        for tail in ("b.n 200 <child>", "bx r3", ".word 0"):
            with self.subTest(tail=tail), self.assertRaises(ValueError):
                audit("100: push {lr}\n102: " + tail)

    def test_unknown_control_transfers_are_rejected(self):
        for transfer in ("bx r3", "mov pc, r2", "ldr pc, [r0]", "cbz r0, 104 <end>"):
            with self.subTest(transfer=transfer), self.assertRaises(ValueError):
                audit("100: " + transfer + "\n102: bx lr\n104: bx lr")

    def test_bounded_jump_table_visits_every_case(self):
        text = """
100: push {lr}
102: cmp r3, #1
104: bhi.n 114 <end>
106: ldr r1, [pc, #24]
108: lsls r2, r3, #2
10a: ldr r2, [r1, r2]
10c: mov pc, r2
10e: sub sp, #16
110: bl 300 <deep_child>
112: add sp, #16
114: pop {pc}
"""
        words = {0x120: 0x200, 0x200: 0x10e, 0x204: 0x114}
        result = audit(text, words.__getitem__)
        self.assertEqual(result["ownPeakBytes"], 20)
        self.assertEqual(result["jumpTables"][0]["targets"], ["0x10e", "0x114"])
        self.assertEqual(result["calls"][0]["target"], "deep_child")
        for bypass in ("104", "106"):
            with self.subTest(bypass=bypass), self.assertRaisesRegex(ValueError, "bypasses"):
                audit(text.replace("110: bl 300 <deep_child>", f"110: b.n {bypass} <unguarded>"), words.__getitem__)


if __name__ == "__main__":
    unittest.main()
