import pathlib, re

def test_no_version_word():
    root = pathlib.Path(__file__).resolve().parents[1]
    banned = []
    for path in root.rglob('*'):
        if path == pathlib.Path(__file__) or '.pytest_cache' in path.parts:
            continue
        if path.is_file() and path.suffix in {'.py', '.md', '.cfg', '.toml', '.json', '.txt'}:
            text = path.read_text(encoding='utf8', errors='ignore')
            if re.search(r'\bversion\b', text, re.IGNORECASE):
                banned.append(str(path))
    assert not banned, f"version word found in: {banned}"
