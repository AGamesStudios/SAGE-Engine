import importlib
import logging
import os
import sys

logger = logging.getLogger('sage_editor')

_PLUGIN_FUNCS = []


def register_plugin(func):
    """Register a plugin initialization function."""
    _PLUGIN_FUNCS.append(func)


def load_plugins(editor, paths=None):
    """Load editor plugins and call them with the editor instance."""
    if paths is None:
        env = os.environ.get('SAGE_EDITOR_PLUGINS')
        paths = env.split(os.pathsep) if env else []

    for func in list(_PLUGIN_FUNCS):
        try:
            func(editor)
        except Exception:
            logger.exception('Plugin function %s failed', getattr(func, '__name__', func))

    for path in paths:
        if not path:
            continue
        if os.path.isdir(path) and path not in sys.path:
            sys.path.append(path)
        for name in os.listdir(path):
            if name.startswith('_') or not name.endswith('.py'):
                continue
            mod_name = os.path.splitext(name)[0]
            try:
                module = importlib.import_module(mod_name)
                init_func = getattr(module, 'init', None)
                if callable(init_func):
                    init_func(editor)
            except Exception:
                logger.exception('Failed to load plugin %s', name)

