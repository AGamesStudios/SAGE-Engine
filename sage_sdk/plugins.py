import importlib
import logging
import os
import sys
import json

logger = logging.getLogger('sage.plugins')

# store plugin functions registered programmatically
_PLUGIN_FUNCS = {
    'engine': [],
    'editor': [],
}

# default plugin directory in the user's home
PLUGIN_DIR = os.path.join(os.path.expanduser('~'), '.sage_plugins')
CONFIG_FILE = os.path.join(PLUGIN_DIR, 'plugins.json')


def read_config():
    """Return the plugin enabled state dictionary."""
    try:
        with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
            if isinstance(data, dict):
                return data
    except Exception:
        pass
    return {}


def write_config(cfg):
    os.makedirs(PLUGIN_DIR, exist_ok=True)
    with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
        json.dump(cfg, f, indent=2)


def list_plugins(paths=None):
    """Return a list of (module name, path, enabled)."""
    dirs = []
    env_all = os.environ.get('SAGE_PLUGINS')
    if env_all:
        dirs.extend(env_all.split(os.pathsep))
    if paths:
        dirs.extend(paths)
    dirs.append(PLUGIN_DIR)

    cfg = read_config()
    result = []
    for path in dirs:
        if not os.path.isdir(path):
            continue
        for name in os.listdir(path):
            if name.startswith('_') or not name.endswith('.py'):
                continue
            mod_name = os.path.splitext(name)[0]
            result.append((mod_name, os.path.join(path, name), cfg.get(mod_name, True)))
    return result


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
    if target == 'engine':
        reg = getattr(module, 'register_logic', None)
        if callable(reg):
            try:
                from engine.logic import register_condition, register_action
                reg(register_condition, register_action)
            except Exception:
                logger.exception('register_logic failed in %s', module.__name__)


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
    dirs.append(PLUGIN_DIR)

    cfg = read_config()

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
            logger.warning('Plugin directory %s does not exist', path)
            continue
        for name in os.listdir(path):
            if name.startswith('_') or not name.endswith('.py'):
                continue
            mod_name = os.path.splitext(name)[0]
            if not cfg.get(mod_name, True):
                continue
            try:
                module = importlib.import_module(mod_name)
                _call_init(module, target, instance)
            except Exception:
                logger.exception('Failed to load plugin %s at %s', mod_name, os.path.join(path, name))

