import json
import os
from glob import glob

from sage_engine.logger import log_crash


def test_log_crash(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    log_crash(ZeroDivisionError, ZeroDivisionError(), None, code="SAGE_ERR_SCRIPT_DIV0")
    files = list(tmp_path.joinpath('logs').glob('crash_*.json'))
    assert files
    data = json.loads(files[0].read_text())
    assert data["code"] == "SAGE_ERR_SCRIPT_DIV0"
