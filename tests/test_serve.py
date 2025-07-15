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
