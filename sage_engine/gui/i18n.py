_translations = {}


def load(src):
    """Load translations from a dict or JSON/YAML file."""
    data = None
    if isinstance(src, str):
        import os, json
        try:
            with open(src, 'r', encoding='utf-8') as f:
                if src.endswith('.json') or src.endswith('.sagei18n'):
                    data = json.load(f)
                else:
                    import yaml
                    data = yaml.safe_load(f)
        except FileNotFoundError:
            data = None
    else:
        data = src
    if isinstance(data, dict):
        _translations.update(data)


def translate(key: str) -> str:
    if key.startswith('@'):
        key = key[1:]
    return _translations.get(key, key)
