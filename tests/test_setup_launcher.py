import sys


def test_install_invokes_pip(monkeypatch):
    import sage_setup.__main__ as setup

    called = {}

    def fake_call(cmd):
        called['cmd'] = cmd

    monkeypatch.setattr(setup, 'REPO_ROOT', '/tmp/repo')
    monkeypatch.setattr(setup.subprocess, 'check_call', fake_call)
    setup.install('/dest', 'sdl')
    assert '--target' in called['cmd']
    assert '/dest' in called['cmd']
    assert 'sdl' in called['cmd'][4]


def test_install_default_path(monkeypatch):
    import sage_setup.__main__ as setup

    called = {}

    def fake_call(cmd):
        called['cmd'] = cmd

    monkeypatch.setattr(setup, 'REPO_ROOT', '/tmp/repo')
    monkeypatch.setattr(setup.subprocess, 'check_call', fake_call)
    monkeypatch.setattr(setup, 'DEFAULT_PATH', '/def')
    setup.install.__defaults__ = ('/def', None)
    setup.install()
    assert '/def' in called['cmd']


def test_launcher_runs_engine(monkeypatch):
    import sage_launcher.__main__ as launcher

    proc = {}

    def fake_popen(cmd):
        proc['cmd'] = cmd

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

