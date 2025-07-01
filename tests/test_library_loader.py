import types
from engine.core.library import LibraryLoader


def test_load_module(tmp_path):
    p = tmp_path / "lib1.py"
    p.write_text("def init_engine(engine): engine.flag = True")
    loader = LibraryLoader(search_paths=[str(tmp_path)])
    engine = types.SimpleNamespace()
    loader.load_all(engine)
    assert 'lib1' in loader.libraries
    assert getattr(engine, 'flag', False)


def test_load_by_name(tmp_path):
    p = tmp_path / "lib2.py"
    p.write_text("value = 42")
    loader = LibraryLoader(search_paths=[str(tmp_path)])
    mod = loader.load('lib2')
    assert mod.value == 42
    assert loader.get('lib2') is mod
