import unittest
from engine.core.object import Object, Transform2D


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


if __name__ == "__main__":
    unittest.main()
