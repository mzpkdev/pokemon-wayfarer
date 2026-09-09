import unittest

from compare_call_trees import closure, compare, decode


class CallTreeTests(unittest.TestCase):
    def test_relocated_calls_keep_closure_and_stack(self):
        base = decode("""00000100 <root>:
 100: push {lr}
 102: bl 200 <child>
 106: pop {pc}
 108: .word 0x08001000
00000200 <child>:
 200: bx lr
""")
        head = decode("""00000300 <root>:
 300: push {lr}
 302: bl 400 <child>
 306: pop {pc}
 308: .word 0x08002000
00000400 <child>:
 400: bx lr
""")
        report = compare(closure(base, ["root"]), closure(head, ["root"]))
        self.assertEqual(report["commonFunctionCount"], 2)
        self.assertEqual(report["controlOrStackChanges"], [])
        self.assertEqual(report["instructionChanges"], {})

    def test_stack_changes_are_reported_and_indirect_roots_are_explicit(self):
        text = """00000100 <root>:
 100: push {lr}
 102: bl 200 <child>
 106: pop {pc}
00000200 <child>:
 200: bx r3
00000300 <callback>:
 300: bx lr
"""
        base = decode(text)
        head = decode(text.replace("push {lr}", "push {r4, lr}"))
        self.assertEqual(set(closure(base, ["root"])), {"root", "child"})
        self.assertIn("callback", closure(base, ["root", "callback"]))
        self.assertEqual(compare(base, head)["controlOrStackChanges"], ["root"])
        with self.assertRaisesRegex(ValueError, "missing"):
            closure(base, ["unresolved"])


if __name__ == "__main__":
    unittest.main()
