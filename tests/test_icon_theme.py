import unittest
from PyQt6.QtGui import QIcon
from PyQt6.QtWidgets import QApplication
from sage_editor.icons import load_icon, set_icon_theme

class TestIconTheme(unittest.TestCase):
    def setUp(self):
        self.app = QApplication.instance() or QApplication([])

    def test_load_black_and_white_icons(self):
        set_icon_theme('black')
        icon_black = load_icon('add.png')
        self.assertIsInstance(icon_black, QIcon)
        self.assertFalse(icon_black.isNull())
        set_icon_theme('white')
        icon_white = load_icon('add.png')
        self.assertIsInstance(icon_white, QIcon)
        self.assertFalse(icon_white.isNull())

if __name__ == '__main__':
    unittest.main()
