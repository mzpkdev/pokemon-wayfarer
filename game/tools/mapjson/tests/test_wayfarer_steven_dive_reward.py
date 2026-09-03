import re
import unittest
from pathlib import Path


GAME = Path(__file__).resolve().parents[3]
SCRIPT = GAME / "data" / "maps" / "MossdeepCity_StevensHouse" / "scripts.inc"
EMERALD_FLAGS = GAME / "include" / "constants" / "flags.h"
WAYFARER_PERSISTENCE = GAME / "include" / "wayfarer_persistence.h"


def label_body(source: str, label: str) -> str:
    match = re.search(rf"(?m)^{re.escape(label)}::?\s*$", source)
    if match is None:
        raise AssertionError(f"missing script label {label}")
    following = source[match.end() :]
    next_label = re.search(r"(?m)^\S[^\n:]*::?\s*$", following)
    return following if next_label is None else following[: next_label.start()]


def assert_order(test: unittest.TestCase, source: str, *needles: str) -> None:
    position = -1
    for needle in needles:
        next_position = source.find(needle, position + 1)
        test.assertNotEqual(next_position, -1, f"missing {needle!r}")
        test.assertGreater(next_position, position, f"{needle!r} is out of order")
        position = next_position


class WayfarerStevenDiveRewardTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = SCRIPT.read_text(encoding="utf-8")

    def test_wayfarer_delivery_precedes_authorization_and_story_commit(self):
        handoff = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_StevenGivesDive"
        )
        assert_order(
            self,
            handoff,
            "#if IS_WAYFARER",
            "checkitem ITEM_HM_DIVE",
            "goto_if_eq VAR_RESULT, TRUE, MossdeepCity_StevensHouse_EventScript_RecognizeDive",
            "additem ITEM_HM_DIVE",
            "goto_if_eq VAR_RESULT, FALSE, MossdeepCity_StevensHouse_EventScript_DiveRewardDeliveryFailed",
            "call EventScript_ObtainItemMessage",
            "goto MossdeepCity_StevensHouse_EventScript_CommitDiveReward",
        )
        self.assertNotIn("FLAG_RECEIVED_HM_DIVE", handoff)
        self.assertNotIn("VAR_STEVENS_HOUSE_STATE, 2", handoff)

        commit = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_CommitDiveReward"
        )
        assert_order(
            self,
            commit,
            "call MossdeepCity_StevensHouse_EventScript_AuthorizeDive",
            "call MossdeepCity_StevensHouse_EventScript_CompleteDiveStory",
        )

        authorization = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_AuthorizeDive"
        )
        self.assertIn("setflag FLAG_RECEIVED_HM_DIVE", authorization)
        self.assertNotIn("additem", authorization)

        story = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_CompleteDiveStory"
        )
        self.assertIn("setvar VAR_STEVENS_HOUSE_STATE, 2", story)
        self.assertNotIn("additem", story)

    def test_received_flag_is_the_dedicated_hoenn_authorization(self):
        emerald_flags = EMERALD_FLAGS.read_text(encoding="utf-8")
        persistence = WAYFARER_PERSISTENCE.read_text(encoding="utf-8")
        self.assertRegex(
            emerald_flags,
            r"(?m)^#define FLAG_RECEIVED_HM_DIVE\s+0x7B\b",
        )
        self.assertIn(
            "#define WAYFARER_HOENN_DIVE_AUTHORIZATION_FLAG HOENN_FLAG_ID(0x07B)",
            persistence,
        )

    def test_owned_hm_reconciles_without_a_duplicate(self):
        recognized = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_RecognizeDive"
        )
        self.assertIn("MossdeepCity_StevensHouse_Text_RecognizedDive", recognized)
        self.assertNotIn("additem", recognized)

        retry = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_RetryDiveReward"
        )
        assert_order(
            self,
            retry,
            "checkitem ITEM_HM_DIVE",
            "goto_if_eq VAR_RESULT, TRUE, MossdeepCity_StevensHouse_EventScript_RetryRecognizeDive",
            "additem ITEM_HM_DIVE",
        )
        retry_recognized = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_RetryRecognizeDive"
        )
        self.assertNotIn("additem", retry_recognized)
        retry_commit = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_CommitRetriedDiveReward"
        )
        self.assertIn(
            "call MossdeepCity_StevensHouse_EventScript_AuthorizeDive", retry_commit
        )
        self.assertIn(
            "call MossdeepCity_StevensHouse_EventScript_CompleteDiveStory", retry_commit
        )

    def test_failed_delivery_stays_pending_and_is_interactively_retryable(self):
        failed = label_body(
            self.source,
            "MossdeepCity_StevensHouse_EventScript_DiveRewardDeliveryFailed",
        )
        self.assertIn("setvar VAR_STEVENS_HOUSE_STATE, 3", failed)
        self.assertNotIn("FLAG_RECEIVED_HM_DIVE", failed)
        self.assertNotIn("VAR_STEVENS_HOUSE_STATE, 2", failed)

        retry_failed = label_body(
            self.source,
            "MossdeepCity_StevensHouse_EventScript_RetryDiveRewardDeliveryFailed",
        )
        self.assertNotIn("FLAG_RECEIVED_HM_DIVE", retry_failed)
        self.assertNotIn("VAR_STEVENS_HOUSE_STATE", retry_failed)

        transition = label_body(self.source, "MossdeepCity_StevensHouse_OnTransition")
        self.assertIn(
            "call_if_eq VAR_STEVENS_HOUSE_STATE, 3, MossdeepCity_StevensHouse_EventScript_SetStevenPos",
            transition,
        )
        steven = label_body(
            self.source, "MossdeepCity_StevensHouse_EventScript_Steven"
        )
        self.assertIn(
            "goto_if_eq VAR_STEVENS_HOUSE_STATE, 3, MossdeepCity_StevensHouse_EventScript_RetryDiveReward",
            steven,
        )

    def test_non_wayfarer_branch_retains_emerald_hm08_handoff(self):
        handoff_start = self.source.index(
            "MossdeepCity_StevensHouse_EventScript_StevenGivesDive::"
        )
        standalone_start = self.source.index("#else", handoff_start)
        standalone_end = self.source.index("#endif", standalone_start)
        standalone = self.source[standalone_start:standalone_end]
        assert_order(
            self,
            standalone,
            "giveitem ITEM_HM_DIVE",
            "setflag FLAG_RECEIVED_HM_DIVE",
            "setvar VAR_STEVENS_HOUSE_STATE, 2",
        )
        self.assertNotIn("additem ITEM_HM_DIVE", standalone)


if __name__ == "__main__":
    unittest.main()
