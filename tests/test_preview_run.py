import os
from sage_engine import preview


def test_preview_run_headless(monkeypatch):
    os.environ["SAGE_HEADLESS"] = "1"
    os.environ["SAGE_PREVIEW_FRAMES"] = "1"
    preview.run()
