import json
import re

__all__ = ['load_json']


def load_json(path: str):
    """Load JSON from ``path`` with a simple trailing comma fixer."""
    with open(path, 'r') as f:
        text = f.read()
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        # remove trailing commas before closing braces/brackets
        cleaned = re.sub(r',\s*([}\]])', r'\1', text)
        return json.loads(cleaned)
