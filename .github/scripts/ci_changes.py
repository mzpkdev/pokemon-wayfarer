"""Conservatively identify documentation-only PRs; uncertainty runs full CI."""

import json
import os
from pathlib import Path
import re
import urllib.request

ROOT_DOCS = {"AGENTS.md", "README.md", "CONTRIBUTING.md", "CHANGELOG.md"}
STATUSES = {"added", "removed", "modified", "renamed", "copied", "changed", "unchanged"}


def documentation_path(path):
    if not isinstance(path, str) or not path or any(
        part in {"", ".", ".."} for part in path.split("/")
    ):
        return False
    return path in ROOT_DOCS or (
        path.startswith((".product/", "docs/")) and path.endswith(".md")
    )


def docs_only(event, get):
    """Require a complete, stable API listing before permitting the cheap path."""
    pr = event["pull_request"]
    number = pr["number"]
    repo = event["repository"]["full_name"]
    if not isinstance(number, int) or number <= 0 or not re.fullmatch(r"[\w.-]+/[\w.-]+", repo):
        return False
    count = pr["changed_files"]
    sha = pr["head"]["sha"]
    base_sha = pr["base"]["sha"]
    if type(count) is not int or not 0 < count <= 3000 or not all(re.fullmatch(r"[0-9a-f]{40}", value) for value in (sha, base_sha)):
        return False
    endpoint = f"/repos/{repo}/pulls/{number}"

    def current():
        metadata = get(endpoint)
        return (
            metadata["head"]["sha"] == sha
            and metadata["base"]["sha"] == base_sha
            and metadata["changed_files"] == count
        )

    if not current():
        return False
    files = []
    for page in range(1, (count + 99) // 100 + 1):
        batch = get(f"{endpoint}/files?per_page=100&page={page}")
        if not isinstance(batch, list) or len(batch) != min(100, count - len(files)):
            return False
        files.extend(batch)
    seen = set()
    for entry in files:
        if not isinstance(entry, dict) or entry.get("status") not in STATUSES:
            return False
        path = entry.get("filename")
        if not documentation_path(path) or path in seen:
            return False
        seen.add(path)
        # Renames/copies must not hide removal of a production input.
        previous = entry.get("previous_filename")
        if entry["status"] in {"renamed", "copied"} and previous is None:
            return False
        if previous is not None and not documentation_path(previous):
            return False
    return len(files) == count and current()


def classify(event_name, event, get):
    if event_name != "pull_request":
        return False
    try:
        return docs_only(event, get)
    except Exception:
        # Authentication, API limits, malformed data, and network failures all
        # retain full validation. Do not print response bodies or credentials.
        print("::warning::Could not verify the PR file list; running full CI.")
        return False


def main():
    def get(endpoint):
        request = urllib.request.Request(
            os.environ.get("GITHUB_API_URL", "https://api.github.com") + endpoint,
            headers={
                "Authorization": "Bearer " + os.environ["GH_TOKEN"],
                "Accept": "application/vnd.github+json",
                "X-GitHub-Api-Version": "2022-11-28",
            },
        )
        with urllib.request.urlopen(request, timeout=30) as response:
            return json.load(response)

    result = False
    try:
        event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text())
        result = classify(os.environ["GITHUB_EVENT_NAME"], event, get)
    except Exception:
        print("::warning::Could not read the PR event; running full CI.")
    with open(os.environ["GITHUB_OUTPUT"], "a") as output:
        output.write(f"docs-only={str(result).lower()}\n")
    print("Documentation-only PR: build the docs UI; skip ROM and browser suites." if result else "Run full CI.")


if __name__ == "__main__":
    main()
