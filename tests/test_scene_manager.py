from engine.core.scenes.manager import SceneManager

class DummyScene:
    def __init__(self):
        self.updated = None
    def update(self, dt):
        self.updated = dt
    def draw(self, renderer):
        pass

def test_scene_manager_switch():
    m = SceneManager()
    s1 = DummyScene()
    s2 = DummyScene()
    m.add_scene("one", s1)
    m.add_scene("two", s2)
    m.set_active("two")
    assert m.get_active_scene() is s2

def test_scene_manager_update():
    m = SceneManager()
    s = DummyScene()
    m.add_scene("main", s)
    m.update(0.5)
    assert s.updated == 0.5
