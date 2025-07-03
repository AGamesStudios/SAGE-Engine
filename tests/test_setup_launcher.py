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


def test_launcher_runs_engine(monkeypatch):
    import sage_launcher.__main__ as launcher

    proc = {}

    def fake_popen(cmd):
        proc['cmd'] = cmd

    monkeypatch.setattr(launcher.subprocess, 'Popen', fake_popen)
    launcher.launch('demo.sageproject')
    assert proc['cmd'][0] == sys.executable
    assert proc['cmd'][-1] == 'demo.sageproject'

