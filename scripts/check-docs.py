#!/usr/bin/env python3
"""Validate repository-relative Markdown references, without network requests."""
from pathlib import Path
import re
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]


def main():
    errors = []
    for document in ROOT.rglob("*.md"):
        relative = document.relative_to(ROOT)
        if relative.parts[0] in {"vendor", ".build", "build", ".git"}:
            continue
        text = document.read_text()
        for target in re.findall(r"!?\[[^\]]*\]\(([^)]+)\)", text):
            target = target.split()[0].strip("<>")
            if re.match(r"(?:https?://|mailto:|#)", target):
                continue
            filename = unquote(target.split("#", 1)[0])
            path = (document.parent / filename).resolve()
            if not path.is_relative_to(ROOT) or not path.exists():
                errors.append(f"{relative}: missing/escaping reference {target}")
    for error in errors:
        print(f"FAIL {error}", file=sys.stderr)
    if errors:
        return 1
    print("PASS local documentation references")
    return 0


if __name__ == "__main__":
    sys.exit(main())
