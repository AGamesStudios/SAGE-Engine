import os
from engine.utils import crash

def test_write_crash_report(tmp_path):
    try:
        raise RuntimeError("boom")
    except RuntimeError as exc:
        path = crash.write_crash_report(RuntimeError, exc, exc.__traceback__, directory=tmp_path)
    assert os.path.exists(path)
    text = open(path, 'r', encoding='utf-8').read()
    assert 'RuntimeError' in text
    assert 'boom' in text
