from __future__ import annotations
import json
from pathlib import Path

LANGUAGES: dict[str, dict[str, str]] = {}
# keys are derived from file names like ``en.json`` -> ``En`` so we
# default to the English dictionary using that key
CURRENT_LANGUAGE = "En"

# map internal language keys to human friendly names which appear in
# the translation files so we can translate menu entries
LANGUAGE_NAMES = {"En": "English", "Ru": "Русский"}


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


def language_label(key: str) -> str:
    """Return the display name for *key* translated for the current locale."""
    name = LANGUAGE_NAMES.get(key, key)
    return tr(name)
