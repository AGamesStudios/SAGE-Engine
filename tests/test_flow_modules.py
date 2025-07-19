from pathlib import Path
from sage_fs import parse_script, FlowRunner


def test_dynamic_module(tmp_path, capsys):
    modules = Path(__file__).resolve().parent.parent / "sage_fs" / "flow_modules"
    mod_file = modules / "say.yaml"
    mod_file.write_text("""\
say:
  args: [text]
  action: print
  aliases: [say]
""", encoding="utf-8")
    try:
        runner = FlowRunner()
        runner.reload_grammar()
        cmds = parse_script('say "hi"')
        runner.execute(cmds)
        out = capsys.readouterr().out
        assert "hi" in out
    finally:
        mod_file.unlink()

