from sage_editor.core import api_bridge


def test_create_and_delete_object():
    api_bridge.create_object()
    objs = api_bridge.get_objects()
    assert objs
    obj_id = objs[-1]["id"]
    api_bridge.delete_object(obj_id)


def test_run_preview_shim(monkeypatch):
    calls = {}

    def fake_run():
        calls["run"] = True

    monkeypatch.setattr("sage_engine.preview.run", fake_run)
    api_bridge.run_preview()
    assert calls.get("run")

