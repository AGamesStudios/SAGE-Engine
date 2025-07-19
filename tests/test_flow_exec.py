from sage import on
from sage_fs import parse_script, FlowRunner


def test_flow_runner_exec(capsys):
    out = []
    on("demo_event", lambda _: out.append("ok"))
    runner = FlowRunner()
    cmds = parse_script('print "hi"\nemit demo_event')
    runner.execute(cmds)
    captured = capsys.readouterr()
    assert "hi" in captured.out
    assert out == ["ok"]
