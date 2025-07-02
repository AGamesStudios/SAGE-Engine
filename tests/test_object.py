import unittest
import types
import sys

# Stub heavy modules so engine.core.scene imports without optional deps
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('engine.renderers', types.ModuleType('engine.renderers'))
sys.modules.setdefault('engine.renderers.shader', types.ModuleType('engine.renderers.shader'))
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))

from dataclasses import dataclass  # noqa: E402

game_mod = types.ModuleType('engine.entities.game_object')

@dataclass
class GameObject:
    name: str = 'GameObject'
    z: int = 0
    def update(self, dt):
        pass
    def draw(self, surface):
        pass

game_mod.GameObject = GameObject
sys.modules.setdefault('engine.entities.game_object', game_mod)

cam_mod = types.ModuleType('engine.core.camera')

@dataclass
class Camera(GameObject):
    active: bool = False

cam_mod.Camera = Camera
sys.modules.setdefault('engine.core.camera', cam_mod)

from engine.entities.object import Object, Transform2D, Material, create_role  # noqa: E402
from engine.core.scenes.scene import Scene  # noqa: E402


class TestObject(unittest.TestCase):
    def test_logic_called(self):
        calls = []

        def logic(obj, dt):
            calls.append((obj.role, dt))

        obj = Object(role="sprite", logic=[logic])
        obj.update(0.5)
        self.assertEqual(calls, [("sprite", 0.5)])

    def test_transform_matrix(self):
        t = Transform2D(x=1, y=2, scale_x=3, scale_y=4, angle=0)
        mat = t.matrix()
        self.assertEqual(len(mat), 9)
        self.assertAlmostEqual(mat[6], 1)
        self.assertAlmostEqual(mat[7], 2)

    def test_object_hierarchy(self):
        parent = Object(role="p")
        child = Object(role="c")
        parent.add_child(child)
        parent.move(5, 2)
        child.move(1, 0)
        wx, wy = child.world_position()
        self.assertAlmostEqual(wx, 6)
        self.assertAlmostEqual(wy, 2)
        self.assertIs(child.parent, parent)
        self.assertIn(child, parent.children)

    def test_scene_object_lookup(self):
        scene = Scene(with_defaults=False)
        obj = Object(role="t")
        scene.add_object(obj)
        by_id = scene.find_object(obj.id)
        self.assertIs(by_id, obj)
        scene.remove_object_by_name(obj.name)
        self.assertIsNone(scene.find_object(obj.id))

    def test_create_role_defaults(self):
        obj = create_role("sprite")
        self.assertEqual(obj.role, "sprite")
        self.assertIsInstance(obj.material, Material)
        cam = create_role("camera", metadata={"active": True})
        self.assertEqual(cam.metadata["width"], 640)
        self.assertTrue(cam.metadata["active"])

    def test_transform_angle_clamped(self):
        t = Transform2D(angle=370)
        self.assertEqual(t.angle, 10)
        t.angle = -45
        self.assertEqual(t.angle, 315)


if __name__ == "__main__":
    unittest.main()

