import pytest

from sage_engine.gui import GuiApp, QtW


def test_import_gui():
    __import__('sage_engine.gui', fromlist=['GuiApp'])
    GuiApp(None)


@pytest.mark.skipif(QtW is None, reason="Qt not installed")
def test_gui_run_headless():
    app = GuiApp(None)
    app.run(headless=True)
