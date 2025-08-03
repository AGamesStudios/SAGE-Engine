import signal

from sage_engine.logger import hooks


def test_signal_handler_keyboard_interrupt(tmp_path, capsys, monkeypatch):
    monkeypatch.chdir(tmp_path)
    monkeypatch.setitem(hooks._prev_signal_hooks, signal.SIGINT, None)
    hooks._handle_signal(signal.SIGINT, None)
    out = capsys.readouterr().out
    assert "invalid traceback" not in out
    assert "Terminated by user" in out
