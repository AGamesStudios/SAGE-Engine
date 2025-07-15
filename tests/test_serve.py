from sage_engine.serve import DummyWS, LiveServer


def test_reload_message(tmp_path):
    server = LiveServer(str(tmp_path))
    ws = DummyWS()
    server.add_client(ws)
    f = tmp_path / "sound.ogg"
    f.write_bytes(b"test")
    server.handle_change(f)
    assert ws.messages
    assert "reload_asset" in ws.messages[0]


def test_theme_reload(tmp_path):
    server = LiveServer(str(tmp_path))
    ws = DummyWS()
    server.add_client(ws)
    f = tmp_path / "dark.vel"
    f.write_text("colors: {}")
    server.handle_change(f)
    assert any("\"type\": \"theme\"" in m for m in ws.messages)
