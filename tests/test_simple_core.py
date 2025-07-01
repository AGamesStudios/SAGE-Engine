from engine.simple_core import Engine, Scene, GameObject

class TestObject(GameObject):
    def __init__(self):
        super().__init__(name="test")
        self.updated = False
    def update(self, dt):
        self.updated = True


def test_scene_update_calls_object_update():
    obj = TestObject()
    scene = Scene()
    scene.add_object(obj)
    scene.update(0.1)
    assert obj.updated


def test_engine_runs_scene():
    scene = Scene()
    engine = Engine(scene=scene, fps=0)
    assert engine.scene is scene
