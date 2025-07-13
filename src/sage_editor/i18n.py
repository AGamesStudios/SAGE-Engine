from __future__ import annotations
import json
from pathlib import Path

LANGUAGES: dict[str, dict[str, str]] = {}
CURRENT_LANGUAGE = "English"


def _load_languages() -> None:
    base = Path(__file__).resolve().parent / "translations"
    for path in base.glob("*.json"):
        try:
            with open(path, "r", encoding="utf-8") as fh:
                LANGUAGES[path.stem.capitalize()] = json.load(fh)
        except Exception:
            LANGUAGES[path.stem.capitalize()] = {}


_load_languages()


def set_language(name: str) -> None:
    global CURRENT_LANGUAGE
    if name in LANGUAGES:
        CURRENT_LANGUAGE = name


def tr(text: str) -> str:
    return LANGUAGES.get(CURRENT_LANGUAGE, {}).get(text, text)
