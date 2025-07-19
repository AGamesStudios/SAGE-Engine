from sage_fs import parse_script, FlowRunner, FlowContext
from sage_engine.object import reset, get_objects


def test_variables_and_arithmetic():
    ctx = FlowContext()
    runner = FlowRunner(context=ctx)
    script = """@global
var score = 1
add score 4
sub score 2
mul score 3
div score 3
"""
    runner.execute(parse_script(script))
    assert ctx.variables["score"] == 3


def test_if_else_and_loop(capsys):
    runner = FlowRunner()
    script = """var x = 0
loop 3:
    add x 1
if x == 3:
    print "done"
else:
    print "fail"
"""
    runner.execute(parse_script(script))
    out = capsys.readouterr().out
    assert "done" in out


def test_create_and_set_object():
    reset()
    runner = FlowRunner()
    script = """create Sprite player
set pos "player" 1 2
"""
    runner.execute(parse_script(script))
    objs = get_objects()
    assert objs[0].id == "player"
    assert objs[0].transform["pos"] == [1.0, 2.0]

