import unittest
from PyQt6.QtWidgets import QApplication
from PyQt6.QtCore import QPoint, Qt
from PyQt6.QtGui import QColor

from sage_paint import PaintWindow, EXPERIMENTAL_NOTICE, Canvas, FillTool, BrushTool


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
        self.assertEqual(color, QColor(255, 255, 255))
        # zoom helper
        before = canvas.zoom_level
        canvas.zoom(2.0)
        self.assertAlmostEqual(canvas.zoom_level, before * 2.0)
        center = canvas.rect().center()
        img_before = canvas.view_to_image(center)
        canvas.zoom_at(center, 1.5)
        self.assertEqual(canvas.view_to_image(center), img_before)

    def test_gizmo_draw(self):
        canvas = Canvas(16, 16)
        from PyQt6.QtGui import QPainter
        painter = QPainter(canvas.image)
        canvas._tool.draw_gizmo(painter, QPoint(0, 0))
        painter.end()

    def test_fill_and_undo(self):
        canvas = Canvas(8, 8)
        canvas.pen_color = QColor('red')
        tool = FillTool(canvas)
        canvas.set_tool(tool)
        tool.press(QPoint(0, 0))
        self.assertEqual(canvas.image.pixelColor(0, 0), QColor('red'))
        canvas.undo()
        self.assertEqual(canvas.image.pixelColor(0, 0), QColor('white'))
        canvas.redo()
        self.assertEqual(canvas.image.pixelColor(0, 0), QColor('red'))

    def test_brush_shapes(self):
        canvas = Canvas(8, 8)
        brush = BrushTool(canvas, shape='square')
        self.assertEqual(brush.pen().capStyle(), Qt.PenCapStyle.SquareCap)

    def test_smooth_toggle(self):
        canvas = Canvas(8, 8)
        self.assertTrue(canvas.smooth_pen)
        canvas.smooth_pen = False
        brush = BrushTool(canvas)
        self.assertFalse(canvas.smooth_pen)


if __name__ == "__main__":
    unittest.main()
