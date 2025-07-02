import sys
import pytest


def test_load_image_error(monkeypatch):
    import importlib
    real_mod = importlib.import_module('engine.entities.game_object')
    monkeypatch.setitem(sys.modules, 'engine.entities.game_object', real_mod)
    real_mod.clear_image_cache()
    GameObject = real_mod.GameObject
    monkeypatch.setattr('PIL.Image.open', lambda p: (_ for _ in ()).throw(FileNotFoundError(p)))
    with pytest.raises(FileNotFoundError):
        GameObject(image_path='definitely_missing.png')

