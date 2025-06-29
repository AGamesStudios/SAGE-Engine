from importlib import import_module

# Re-export icons from the editor package so other modules can use the same set
# without depending on ``sage_editor`` directly.

_editor_icons = import_module('sage_editor.icons')

load_icon = _editor_icons.load_icon
app_icon = _editor_icons.app_icon
ICON_DIR = _editor_icons.ICON_DIR
APP_ICON_NAME = _editor_icons.APP_ICON_NAME
