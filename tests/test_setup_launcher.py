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
            self.stderr = io.StringIO()

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
            self.stderr = io.StringIO()
            self.stdout = io.StringIO()

        def wait(self):
            return 0

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc(cmd)

    monkeypatch.setattr(setup, 'REPO_ROOT', '/tmp/repo')
    monkeypatch.setattr(setup.subprocess, 'Popen', fake_popen)
    monkeypatch.setattr(setup, 'DEFAULT_PATH', '/def')
    setup.install.__defaults__ = ('/def', None, None)
    setup.install()
    assert '/def' in called['cmd']


def test_install_with_version(monkeypatch):
    import sage_setup.__main__ as setup

    called = {}

    class DummyProc:
        def __init__(self, cmd):
            called['cmd'] = cmd
            self.stderr = io.StringIO()
            self.returncode = 0
            self.stdout = io.StringIO()

        def wait(self):
            return 0

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc(cmd)

    monkeypatch.setattr(setup.subprocess, 'Popen', fake_popen)
    setup.install('/base', version='1.2')
    assert 'sage-engine==1.2' in called['cmd']
    assert '/base/1.2' in called['cmd']


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
    launcher.create_project(str(p), version="1.0")
    assert p.exists()
    import json
    data = json.loads(p.read_text())
    assert data["metadata"]["engine_version"] == "1.0"


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
            self.stderr = io.StringIO()
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
            self.stderr = io.StringIO("E")
            self.returncode = 1

        def wait(self):
            return 1

    def fake_popen(cmd, stdout=None, stderr=None, text=False):
        return DummyProc()

    monkeypatch.setattr(setup.subprocess, "Popen", fake_popen)
    with pytest.raises(RuntimeError) as exc:
        setup.install("/t")
    assert "err" in str(exc.value)


def test_installed_versions(tmp_path, monkeypatch):
    import sage_setup.__main__ as setup

    v1 = tmp_path / "1.0"
    v2 = tmp_path / "2.0"
    v1.mkdir()
    v2.mkdir()
    monkeypatch.setattr(setup, "DEFAULT_PATH", str(tmp_path))
    assert setup.installed_versions(str(tmp_path)) == ["1.0", "2.0"]


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
            self.text = DummyText()
            self.bar = DummyBar()

        def setWindowTitle(self, t):
            pass

        def setWindowModality(self, m):
            pass

        def setFixedSize(self, w, h):
            pass

        class Rect:
            def moveCenter(self, p):
                pass

            def topLeft(self):
                return 0

        def frameGeometry(self):
            return self.Rect()

        def move(self, p):
            pass

        def show(self):
            pass

        def close(self):
            pass

    class DummyText:
        def __init__(self):
            self.data = ""

        def setReadOnly(self, b):
            pass

        def appendPlainText(self, t):
            self.data += t + "\n"
            outputs.append(self.data)

        def toPlainText(self):
            return self.data

    class DummyBar:
        def setRange(self, a, b):
            pass

        def setValue(self, v):
            pass

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
    qtwidgets.QDialog = DummyDialog
    qtwidgets.QPlainTextEdit = DummyText
    qtwidgets.QProgressBar = DummyBar
    class DummyLayout:
        def __init__(self, *a, **k):
            pass

        def addWidget(self, w):
            pass

    qtwidgets.QVBoxLayout = DummyLayout
    qtwidgets.QApplication = DummyApp
    qtwidgets.QMessageBox = DummyMsgBox
    class DummyWidget:
        class Rect:
            def center(self):
                return 0

            def moveCenter(self, p):
                pass

            def topLeft(self):
                return 0

        def frameGeometry(self):
            return self.Rect()

    qtwidgets.QWidget = DummyWidget
    qtcore = types.ModuleType("PyQt6.QtCore")
    qtcore.Qt = types.SimpleNamespace(WindowModality=types.SimpleNamespace(WindowModal=0))
    monkeypatch.setitem(sys.modules, "PyQt6.QtWidgets", qtwidgets)
    monkeypatch.setitem(sys.modules, "PyQt6.QtCore", qtcore)

    def fake_install_iter(path, extras, version, *, launcher_only=False):
        yield "line1\n", 10
        yield "line2\n", 100

    monkeypatch.setattr(setup, "install_iter", fake_install_iter)
    txt = setup.run_install_dialog(None, None, None, qtwidgets.QWidget())
    assert "line1" in txt and "line2" in txt


def test_run_setup_starts_process(monkeypatch):
    import sage_launcher.__main__ as launcher

    called = {}

    def fake_popen(cmd):
        called["cmd"] = cmd
        return None

    monkeypatch.setattr(launcher.subprocess, "Popen", fake_popen)
    launcher.run_setup()
    assert called["cmd"] == [sys.executable, "-m", "sage_setup"]


def test_open_docs(monkeypatch):
    import sage_launcher.__main__ as launcher

    urls = []

    def fake_open(url):
        urls.append(url)
        return True

    monkeypatch.setattr(launcher.webbrowser, "open", fake_open)
    launcher.open_docs()
    assert urls[0] == launcher.DOC_URL


def test_open_docs_local(monkeypatch, tmp_path):
    import sage_launcher.__main__ as launcher

    urls = []

    def fake_open(url):
        urls.append(url)
        return True

    monkeypatch.setattr(launcher.webbrowser, "open", fake_open)
    doc = tmp_path / "index.html"
    doc.write_text("hi")
    monkeypatch.setenv("SAGE_DOC_PATH", str(doc))
    launcher.open_docs()
    assert urls[-1].startswith("file://")


def test_translation_env(monkeypatch):
    monkeypatch.setenv("SAGE_LANG", "ru")
    import importlib
    launcher = importlib.reload(__import__("sage_launcher.__main__", fromlist=["tr"]))
    assert launcher.tr("open").startswith("О")

