from sage_sdk.plugins import register_plugin as _reg, load_plugins as _load

def register_plugin(func):
    """Register an editor plugin programmatically."""
    _reg('editor', func)


def load_plugins(editor, paths=None):
    """Load editor plugins and call them with the editor instance."""
    _load('editor', editor, paths)

