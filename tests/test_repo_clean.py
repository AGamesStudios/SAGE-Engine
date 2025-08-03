from pathlib import Path
import re

FORBIDDEN = {"version", "schema_version", "api_version"}


def test_forbidden_keywords():
    root = Path(__file__).resolve().parents[1]
    for path in root.rglob("*"):
        if (
            ".git" in path.parts
            or "__pycache__" in path.parts
            or ".pytest_cache" in path.parts
            or path.name in {"test_repo_clean.py", "test_no_versions.py"}
        ):
            continue
        if path.is_file() and path.suffix not in {".ttf", ".lock", ".svg", ".pyc"}:
            text = path.read_text(encoding="utf8", errors="ignore")
            for word in FORBIDDEN:
                assert not re.search(rf"\b{word}\b", text, re.IGNORECASE), f"{word} found in {path}"
