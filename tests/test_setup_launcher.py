import sys
import io
import pytest


def test_install_invokes_pip(monkeypatch):
    import sage_setup.__main__ as setup

    called = {}

    class DummyProc:
        def __init__(self, cmd):
            called['cmd'] = cmd
            self.returncode = 0
            self.stdout = io.StringIO()

        def wait(self):
            return 0

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc(cmd)

    monkeypatch.setattr(setup, 'REPO_ROOT', '/tmp/repo')
    monkeypatch.setattr(setup.subprocess, 'Popen', fake_popen)
    setup.install('/dest', 'sdl')
    assert '--target' in called['cmd']
    assert '/dest' in called['cmd']
    assert 'sdl' in called['cmd'][4]


def test_install_default_path(monkeypatch):
    import sage_setup.__main__ as setup

    called = {}

    class DummyProc:
        def __init__(self, cmd):
            called['cmd'] = cmd
            self.returncode = 0
            self.stdout = io.StringIO()

        def wait(self):
            return 0

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc(cmd)

    monkeypatch.setattr(setup, 'REPO_ROOT', '/tmp/repo')
    monkeypatch.setattr(setup.subprocess, 'Popen', fake_popen)
    monkeypatch.setattr(setup, 'DEFAULT_PATH', '/def')
    setup.install.__defaults__ = ('/def', None)
    setup.install()
    assert '/def' in called['cmd']


def test_launcher_runs_engine(monkeypatch):
    import sage_launcher.__main__ as launcher

    proc = {}

    class DummyProc:
        def __init__(self, cmd):
            proc['cmd'] = cmd

    def fake_popen(cmd, start_new_session=True):
        return DummyProc(cmd)

    monkeypatch.setattr(launcher.subprocess, 'Popen', fake_popen)
    launcher.launch('demo.sageproject')
    assert proc['cmd'][0] == sys.executable
    assert proc['cmd'][-1] == 'demo.sageproject'


def test_project_listing(tmp_path):
    import sage_launcher.__main__ as launcher

    a = tmp_path / 'a.sageproject'
    b = tmp_path / 'sub' / 'b.sageproject'
    b.parent.mkdir()
    a.write_text('{}')
    b.write_text('{}')
    found = launcher.list_projects(tmp_path)
    assert str(a) in found and str(b) in found


def test_create_project(tmp_path):
    import sage_launcher.__main__ as launcher

    p = tmp_path / 'new.sageproject'
    launcher.create_project(str(p))
    assert p.exists()


def test_available_extras_pyproject(tmp_path, monkeypatch):
    import sage_setup.__main__ as setup

    py = tmp_path / "pyproject.toml"
    py.write_text("""[project.optional-dependencies]\nfoo = []\nbar = []\n""")
    monkeypatch.setattr(setup, "REPO_ROOT", str(tmp_path))
    assert setup.available_extras() == ["bar", "foo"]


def test_available_extras_metadata(monkeypatch):
    import sage_setup.__main__ as setup
    class DummyMsg:
        def get_all(self, key):
            return ["baz", "foo"] if key == "Provides-Extra" else []

    monkeypatch.setattr(setup, "REPO_ROOT", "/not/exist")
    import importlib.metadata as md
    monkeypatch.setattr(md, "metadata", lambda name: DummyMsg())
    assert setup.available_extras() == ["baz", "foo"]


def test_install_no_target(monkeypatch):
    import sage_setup.__main__ as setup

    called = {}

    class DummyProc:
        def __init__(self, cmd):
            called["cmd"] = cmd
            self.returncode = 0
            self.stdout = io.StringIO()

        def wait(self):
            return 0

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc(cmd)

    monkeypatch.setattr(setup.subprocess, "Popen", fake_popen)
    setup.install(None)
    assert "--target" not in called["cmd"]


def test_install_error_output(monkeypatch):
    import sage_setup.__main__ as setup

    class DummyProc:
        def __init__(self):
            self.stdout = io.StringIO("out\nerr")
            self.returncode = 1

        def wait(self):
            return 1

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc()

    monkeypatch.setattr(setup.subprocess, "Popen", fake_popen)
    with pytest.raises(RuntimeError) as exc:
        setup.install("/t")
    assert "err" in str(exc.value)


def test_list_projects_non_recursive(tmp_path):
    import sage_launcher.__main__ as launcher

    a = tmp_path / "a.sageproject"
    b = tmp_path / "sub" / "b.sageproject"
    b.parent.mkdir()
    a.write_text("{}")
    b.write_text("{}")
    found = launcher.list_projects(tmp_path, recursive=False)
    assert str(a) in found and str(b) not in found


def test_settings_round_trip(monkeypatch):
    import sage_launcher.__main__ as launcher

    store = {}

    class DummySettings:
        def __init__(self, org, name):
            self._org = org
            self._name = name

        def value(self, key, default=None, type=None):
            return store.get(key, default)

        def setValue(self, key, value):
            store[key] = value

    monkeypatch.setattr(launcher, "QSettings", DummySettings)
    launcher.save_last_dir("/x")
    launcher.save_last_dir("/y")
    assert launcher.load_last_dir() == "/y"
    assert launcher.load_recent_dirs() == ["/y", "/x"]


def test_install_dialog_appends_output(monkeypatch):
    import types
    import sage_setup.__main__ as setup

    outputs = []

    class DummyDialog:
        def __init__(self, *a, **k):
            self._text = ""

        def setWindowTitle(self, t):
            pass

        def setWindowModality(self, m):
            pass

        def show(self):
            pass

        def close(self):
            pass

        def labelText(self):
            return self._text

        def setLabelText(self, t):
            self._text = t
            outputs.append(t)

    class DummyApp:
        def __init__(self, *a, **k):
            pass

        def setStyle(self, s):
            pass

        def exec(self):
            pass

        @staticmethod
        def processEvents():
            pass

    class DummyMsgBox:
        @staticmethod
        def critical(*a):
            pass

        @staticmethod
        def information(*a):
            pass

    monkeypatch.setitem(sys.modules, "PyQt6", types.ModuleType("PyQt6"))
    qtwidgets = types.ModuleType("PyQt6.QtWidgets")
    qtwidgets.QProgressDialog = DummyDialog
    qtwidgets.QApplication = DummyApp
    qtwidgets.QMessageBox = DummyMsgBox
    qtwidgets.QWidget = type("QWidget", (), {})
    qtcore = types.ModuleType("PyQt6.QtCore")
    qtcore.Qt = types.SimpleNamespace(WindowModality=types.SimpleNamespace(WindowModal=0))
    monkeypatch.setitem(sys.modules, "PyQt6.QtWidgets", qtwidgets)
    monkeypatch.setitem(sys.modules, "PyQt6.QtCore", qtcore)

    def fake_install_iter(path, extras):
        yield "line1\n"
        yield "line2\n"

    monkeypatch.setattr(setup, "install_iter", fake_install_iter)
    txt = setup.run_install_dialog(None, None, qtwidgets.QWidget())
    assert "line1" in txt and "line2" in txt

