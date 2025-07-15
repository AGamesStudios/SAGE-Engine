import argparse
import sys

sys.path.insert(0, "src")

import sage_cli
import sage_engine.render as render


def test_backend_select(monkeypatch, tmp_path, capsys):
    script = tmp_path / "dummy.py"
    script.write_text("from sage_engine import render; render.get_backend()\n")

    class FakeEP:
        def __init__(self, name):
            self.name = name
        def load(self):
            from sage_render_headless.backend import HeadlessBackend
            return HeadlessBackend

    monkeypatch.setattr(render.metadata, "entry_points", lambda group=None: [FakeEP("headless")])
    args = argparse.Namespace(cmd="run", script=str(script), gui="auto", render="auto")
    sage_cli.cmd_run(args)
    out = capsys.readouterr().out
    assert "Render backend: headless" in out
