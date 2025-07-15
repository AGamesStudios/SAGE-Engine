from sage_engine.core.scenes.scene import Scene
from sage_engine.formats import save_sagelogic, load_sagelogic


def test_sagelogic_roundtrip(tmp_path):
    path = tmp_path / "logic.sagelogic"
    events = [{"name": "A", "conditions": [], "actions": []}]
    save_sagelogic(events, path)
    loaded = load_sagelogic(path)
    assert loaded[0]["name"] == "A"
    scene = Scene.from_dict({"logic_files": [str(path)]}, base_path=str(tmp_path))
    assert scene.events[0]["name"] == "A"


def test_logic_script(tmp_path):
    logic_script = tmp_path / "script.py"
    logic_script.write_text(
        "def register(scene):\n    scene.events.append({'name':'B','conditions':[],'actions':[]})\n"
    )
    scene = Scene.from_dict({"logic_scripts": [str(logic_script)]}, base_path=str(tmp_path))
    assert any(evt["name"] == "B" for evt in scene.events)


def test_sagelogic_with_comments(tmp_path):
    path = tmp_path / "logic.sagelogic"
    path.write_text(
        """
        {
          // example event
          "events": [
            {"name": "C", "conditions": [], "actions": []},
          ]
        }
        """
    )
    events = load_sagelogic(path)
    assert events[0]["name"] == "C"
