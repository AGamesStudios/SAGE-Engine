from sage_fs import parse_script


def test_parse_script_synonyms():
    script = "сказать \"Привет\"\nemit event"
    cmds = parse_script(script)
    assert len(cmds) == 2
    assert cmds[0].cmd == "print" and cmds[0].args == ["Привет"]
    assert cmds[1].cmd == "emit" and cmds[1].args == ["event"]
