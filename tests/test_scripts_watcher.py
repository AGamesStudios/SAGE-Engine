from sage_engine.scripts_watcher import ScriptsWatcher
from sage_engine.object import reset


def test_scripts_watcher(tmp_path, capsys):
    reset()
    fs = tmp_path / "hello.sage_fs"
    fs.write_text('print "FS"', encoding='utf-8')
    lua = tmp_path / "hello.lua"
    lua.write_text("print('LUA')", encoding='utf-8')
    watcher = ScriptsWatcher(folder=str(tmp_path))
    capsys.readouterr()
    watcher.scan()
    out = capsys.readouterr().out
    assert "FS" in out

    fs.write_text('print "NEW"', encoding='utf-8')
    watcher.scan()
    out2 = capsys.readouterr().out
    assert "NEW" in out2
