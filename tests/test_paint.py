import unittest
from PyQt6.QtWidgets import QApplication

from sage_paint import PaintWindow, EXPERIMENTAL_NOTICE


class TestPaintModule(unittest.TestCase):
    def setUp(self):
        self.app = QApplication.instance() or QApplication([])

    def test_experimental_notice(self):
        self.assertIn("experimental", EXPERIMENTAL_NOTICE.lower())

    def test_window_title(self):
        win = PaintWindow()
        self.assertTrue(win.windowTitle().startswith("SAGE Paint"))


if __name__ == "__main__":
    unittest.main()
