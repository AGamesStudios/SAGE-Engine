from sage_fs import parse_script


def test_parse_script_synonyms():
    script = "сказать \"Привет\"\nemit event"
    cmds = parse_script(script)
    assert cmds == [("print", '"Привет"'), ("emit", "event")]
