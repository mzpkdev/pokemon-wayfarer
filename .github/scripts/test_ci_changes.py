import copy
import unittest
from unittest.mock import Mock

from ci_changes import classify, documentation_path


class ClassificationTests(unittest.TestCase):
    def setUp(self):
        self.event = {
            "repository": {"full_name": "owner/repo"},
            "pull_request": {"number": 12, "changed_files": 1, "head": {"sha": "a" * 40}, "base": {"sha": "c" * 40}},
        }

    def check(self, files, *, count=None, metadata=None):
        self.event["pull_request"]["changed_files"] = len(files) if count is None else count
        state = metadata or self.event["pull_request"]
        def get(endpoint):
            if "/files?" in endpoint:
                page = int(endpoint.rsplit("=", 1)[1])
                return files[(page - 1) * 100:page * 100]
            return state
        api = Mock(side_effect=get)
        return classify("pull_request", self.event, api), api

    def test_allowlist(self):
        for path in [".product/prds/a.md", ".product/specs/deep/a.md", "docs/a.md", "AGENTS.md", "README.md", "CONTRIBUTING.md", "CHANGELOG.md"]:
            with self.subTest(path=path):
                self.assertTrue(documentation_path(path))
        for path in ["game/README.md", ".github/README.md", "docs/a.json", "docs/../game/a.md", "/docs/a.md", "docs//a.md", "unknown.md", "docs/a.MD", None]:
            with self.subTest(path=path):
                self.assertFalse(documentation_path(path))

    def test_mixed_paths_run_full(self):
        result, _ = self.check([{"filename": "docs/a.md", "status": "modified"}, {"filename": "game/src/a.c", "status": "modified"}])
        self.assertFalse(result)

    def test_document_deletions_are_allowed(self):
        result, _ = self.check([{"filename": ".product/specs/old.md", "status": "removed"}])
        self.assertTrue(result)

    def test_code_deletions_run_full(self):
        result, _ = self.check([{"filename": "game/src/a.c", "status": "removed"}])
        self.assertFalse(result)

    def test_rename_requires_both_paths(self):
        for previous, expected in [("game/src/a.c", False), ("docs/old.md", True), (None, False)]:
            with self.subTest(previous=previous):
                result, _ = self.check([{"filename": "docs/new.md", "status": "renamed", "previous_filename": previous}])
                self.assertEqual(result, expected)
        result, _ = self.check([{"filename": "game/a.c", "status": "renamed", "previous_filename": "docs/old.md"}])
        self.assertFalse(result)

    def test_more_than_100_files(self):
        files = [{"filename": f"docs/{i}.md", "status": "added"} for i in range(205)]
        result, api = self.check(files)
        self.assertTrue(result)
        self.assertEqual(api.call_count, 5)  # Three pages, metadata before and after.
        files[-1]["filename"] = "game/src/a.c"
        self.assertFalse(self.check(files)[0])

    def test_empty_truncated_and_over_limit(self):
        self.assertFalse(self.check([])[0])
        files = [{"filename": f"docs/{i}.md", "status": "added"} for i in range(100)]
        self.assertFalse(self.check(files, count=101)[0])
        self.assertFalse(self.check(files, count=3001)[0])

    def test_sha_or_count_mismatch(self):
        for key, value in [("head", {"sha": "b" * 40}), ("base", {"sha": "d" * 40}), ("changed_files", 2)]:
            metadata = copy.deepcopy(self.event["pull_request"])
            metadata[key] = value
            self.assertFalse(self.check([{"filename": "docs/a.md", "status": "added"}], metadata=metadata)[0])

    def test_head_changes_during_pagination(self):
        state = copy.deepcopy(self.event["pull_request"])
        newer = copy.deepcopy(state)
        newer["head"]["sha"] = "b" * 40
        api = Mock(side_effect=[state, [{"filename": "docs/a.md", "status": "added"}], newer])
        self.assertFalse(classify("pull_request", self.event, api))

    def test_api_errors_and_malformed_data(self):
        for response in [RuntimeError("rate limited"), {}, None, "invalid"]:
            with self.subTest(response=response):
                api = Mock(side_effect=response) if isinstance(response, Exception) else Mock(return_value=response)
                self.assertFalse(classify("pull_request", self.event, api))
        for files in [[{}], [{"filename": "docs/a.md", "status": "unexpected"}], [None], [{"filename": "docs/a.md", "status": "added"}] * 2]:
            self.assertFalse(self.check(files)[0])
        self.assertFalse(classify("pull_request", {}, Mock()))

    def test_pagination_api_failure(self):
        self.event["pull_request"]["changed_files"] = 101
        files = [{"filename": f"docs/{i}.md", "status": "added"} for i in range(100)]
        api = Mock(side_effect=[self.event["pull_request"], files, RuntimeError("API unavailable")])
        self.assertFalse(classify("pull_request", self.event, api))

    def test_push_and_manual_never_call_api(self):
        for event_name in ["push", "workflow_dispatch"]:
            api = Mock()
            self.assertFalse(classify(event_name, self.event, api))
            api.assert_not_called()


if __name__ == "__main__":
    unittest.main()
