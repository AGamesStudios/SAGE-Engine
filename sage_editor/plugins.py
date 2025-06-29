from sage_sdk.plugins import PluginManager

EDITOR_PLUGINS = PluginManager('editor')

def register_plugin(func):
    """Register an editor plugin programmatically."""
    EDITOR_PLUGINS.register(func)


def load_plugins(editor, paths=None):
    """Load editor plugins and call them with the editor instance."""
    EDITOR_PLUGINS.load(editor, paths)

