import unittest
from PyQt6.QtWidgets import QApplication

from sage_paint import PaintWindow, EXPERIMENTAL_NOTICE, Canvas


class TestPaintModule(unittest.TestCase):
    def setUp(self):
        self.app = QApplication.instance() or QApplication([])

    def test_experimental_notice(self):
        self.assertIn("experimental", EXPERIMENTAL_NOTICE.lower())

    def test_window_title(self):
        win = PaintWindow()
        self.assertTrue(win.windowTitle().startswith("SAGE Paint"))

    def test_canvas_defaults(self):
        canvas = Canvas(16, 16)
        # Top-left pixel should be white
        color = canvas.image.pixelColor(0, 0)
        from PyQt6.QtGui import QColor
        self.assertEqual(color, QColor(255, 255, 255))
        # zoom helper
        before = canvas.zoom_level
        canvas.zoom(2.0)
        self.assertAlmostEqual(canvas.zoom_level, before * 2.0)


if __name__ == "__main__":
    unittest.main()
