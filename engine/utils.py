import json
import re

__all__ = ['load_json']


def load_json(path: str):
    """Load JSON from ``path`` while tolerating comments and trailing commas."""
    with open(path, 'r', encoding='utf-8') as f:
        text = f.read()

    # Strip // and # comments as well as /* */ blocks
    cleaned = re.sub(r"//.*?$|#.*?$|/\*.*?\*/", "", text, flags=re.MULTILINE | re.DOTALL)
    # Remove trailing commas before closing brackets or braces
    cleaned = re.sub(r',\s*([}\]])', r'\1', cleaned)

    return json.loads(cleaned)
