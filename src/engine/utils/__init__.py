import json
import re

from .crash import write_crash_report

__all__ = ['load_json', 'write_crash_report']


def load_json(path: str):
    """Load JSON from ``path`` while tolerating comments and trailing commas."""
    with open(path, "r", encoding="utf-8") as f:
        text = f.read()

    # Strip // and # comments as well as /* */ blocks
    cleaned = re.sub(
        r"//.*?$|#.*?$|/\*.*?\*/",
        "",
        text,
        flags=re.MULTILINE | re.DOTALL,
    )
    # Remove trailing commas before closing brackets or braces
    cleaned = re.sub(r",\s*([}\]])", r"\1", cleaned)

    if not cleaned.strip():
        return {}

    try:
        data = json.loads(cleaned)
    except json.JSONDecodeError as exc:
        raise ValueError(f"Invalid JSON in {path!r}: {exc}") from exc
    if not isinstance(data, dict):
        raise ValueError(f"Invalid JSON structure in {path!r}: expected object")
    return data
