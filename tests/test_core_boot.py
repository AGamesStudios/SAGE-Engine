import os
os.environ["SDL_VIDEODRIVER"] = "dummy"

from sage_engine import core_boot, core_reset, core_debug
from sage_engine.object import get_objects


def _write_ui_object(folder: str) -> None:
    os.makedirs(folder, exist_ok=True)
    path = os.path.join(folder, "ui.sage_object")
    with open(path, "w", encoding="utf-8") as fh:
        fh.write('{"role": "UI"}')


def test_core_boot_and_reset(tmp_path, monkeypatch):
    obj_folder = tmp_path / "data" / "objects"
    _write_ui_object(str(obj_folder))
    with monkeypatch.context() as m:
        m.chdir(tmp_path)
        profile = core_boot()
        assert profile.entries
        assert any(obj.role == "UI" for obj in get_objects())
        profile2 = core_reset()
        assert profile2.entries
        core_debug()
