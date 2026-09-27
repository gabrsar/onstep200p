#!/usr/bin/env python3
"""Check publishable working-tree candidates without printing sensitive values."""
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
ALLOWED = {".editorconfig", ".gitignore", ".gitmodules", ".github", "README.md",
           "LICENSE", "NOTICE.md", "CHANGELOG.md", "CONTRIBUTING.md", "SECURITY.md",
           "Makefile", "config", "plugins", "scripts", "tests", "docs", "vendor"}
PATTERNS = [
    ("personal filesystem path", re.compile(r"/(?:Users|home)/[A-Za-z0-9_-]+/")),
    ("private key", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----")),
    ("GitHub token", re.compile(r"(?:gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{30,})")),
    ("cloud access key", re.compile(r"\bAKIA[A-Z0-9]{16}\b")),
    ("device MAC address", re.compile(r"\b(?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}\b")),
]


def main():
    candidates = subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"], cwd=ROOT
    ).decode().split("\0")
    problems = []
    for name in sorted(set(filter(None, candidates))):
        path = Path(name)
        if path.parts[0] not in ALLOWED:
            problems.append((name, "outside public project allowlist"))
        if path.parts[0] == "vendor":
            if name != "vendor/OnStepX":
                problems.append((name, "upstream must remain a submodule"))
            continue
        if ".local." in name or path.suffix in {".bin", ".elf", ".pem", ".key", ".log"}:
            problems.append((name, "private/generated artifact"))
        full = ROOT / path
        if full.is_symlink():
            problems.append((name, "unexpected symlink"))
            continue
        if not full.is_file():
            continue
        try:
            content = full.read_text()
        except UnicodeDecodeError:
            problems.append((name, "unreviewed binary file"))
            continue
        for description, pattern in PATTERNS:
            if pattern.search(content):
                problems.append((name, description))
        # Only placeholders belong in a committed station credential example.
        for password in re.findall(r'^\s*#define\s+STA_PASSWORD\s+"([^"\n]*)"', content, re.M):
            if password != "YOUR_WIFI_PASSWORD":
                problems.append((name, "station password literal"))
        if name.startswith(".github/workflows/"):
            if "pull_request_target" in content:
                problems.append((name, "privileged PR workflow"))
            for line in content.splitlines():
                if "uses:" in line and not re.search(r"uses:\s+[\w.-]+/[\w.-]+@[0-9a-f]{40}\b", line):
                    problems.append((name, "action is not pinned by commit"))
    if problems:
        for name, reason in problems:
            print(f"FAIL {name}: {reason}", file=sys.stderr)
        return 1
    print("PASS public candidate allowlist and sensitive-pattern audit")
    return 0


if __name__ == "__main__":
    sys.exit(main())
