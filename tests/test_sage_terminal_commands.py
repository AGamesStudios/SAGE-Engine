from importlib import import_module
from pathlib import Path

commands = import_module('tools.sage_terminal.commands')


class DummyApp:
    def close(self) -> None:
        pass


def test_newproject(tmp_path: Path) -> None:
    target = tmp_path / 'proj'
    out = commands.cmd_newproject(DummyApp(), str(target))
    assert '[\u2713]' in out
    assert (target / 'main.py').exists()


def test_execute_unknown() -> None:
    out = commands.execute(DummyApp(), 'foo')
    assert out == 'unknown command'
