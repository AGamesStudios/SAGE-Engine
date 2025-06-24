import importlib
import logging
import os
import sys

logger = logging.getLogger('sage.plugins')

# store plugin functions registered programmatically
_PLUGIN_FUNCS = {
    'engine': [],
    'editor': [],
}


def register_plugin(target: str, func):
    """Register a plugin for the given target ('engine' or 'editor')."""
    if target not in _PLUGIN_FUNCS:
        raise ValueError(f'Unknown plugin target: {target}')
    _PLUGIN_FUNCS[target].append(func)


def _call_init(module, target, instance):
    """Call the init function for the requested target if present."""
    init_name = f'init_{target}'
    func = getattr(module, init_name, None)
    if callable(func):
        func(instance)
    else:
        generic = getattr(module, 'init', None)
        if callable(generic):
            generic(instance)


def load_plugins(target: str, instance, paths=None):
    """Load plugins for the given target and initialize them with *instance*."""
    if target not in _PLUGIN_FUNCS:
        raise ValueError(f'Unknown plugin target: {target}')

    dirs = []
    env_all = os.environ.get('SAGE_PLUGINS')
    if env_all:
        dirs.extend(env_all.split(os.pathsep))
    env_specific = os.environ.get('SAGE_EDITOR_PLUGINS' if target == 'editor' else 'SAGE_ENGINE_PLUGINS')
    if env_specific:
        dirs.extend(env_specific.split(os.pathsep))
    if paths:
        dirs.extend(paths)

    for func in list(_PLUGIN_FUNCS[target]):
        try:
            func(instance)
        except Exception:
            logger.exception('Plugin function %s failed', getattr(func, '__name__', func))

    for path in dirs:
        if not path:
            continue
        if os.path.isdir(path) and path not in sys.path:
            sys.path.append(path)
        if not os.path.isdir(path):
            continue
        for name in os.listdir(path):
            if name.startswith('_') or not name.endswith('.py'):
                continue
            mod_name = os.path.splitext(name)[0]
            try:
                module = importlib.import_module(mod_name)
                _call_init(module, target, instance)
            except Exception:
                logger.exception('Failed to load plugin %s', name)

