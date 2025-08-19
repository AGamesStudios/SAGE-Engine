#!/usr/bin/env python3
import argparse, os

from sagecore.core.plugins import PluginManager

# --- SAGE Plugin enable/disable integration
import json as _json, os as _os
from pathlib import Path as _Path
_ENABLED = []
try:
    _ep = _Path('docs/plugins/enabled_plugins.json')
    if _ep.exists():
        _ENABLED = _json.loads(_ep.read_text(encoding='utf-8'))
except Exception:
    _ENABLED = []
# Allow override via env SAGE_INCLUDE_PLUGINS
if _os.getenv('SAGE_INCLUDE_PLUGINS'):
    _ENABLED = [p for p in _os.getenv('SAGE_INCLUDE_PLUGINS','').split(',') if p]

try:
    # If available
    from sagecore.core.plugins import PluginLoadConfig
except Exception:
    class PluginLoadConfig:
        def __init__(self, profile='auto', include=None, exclude=None):
            self.profile = profile
            self.include = set(include or [])
            self.exclude = set(exclude or [])

from sagecore.core.engine import Engine

def make_pm(plugins_dir, cfg):
    # Try new signature first
    try:
        return PluginManager(plugins_dir, config=cfg)
    except TypeError:
        pm = PluginManager(plugins_dir)  # old signature
        # Retro‑fit helpful attributes/methods for older cores
        try:
            pm.config = cfg
        except Exception:
            pass
        if not hasattr(pm, "get_services"):
            def _get_services(name):
                # Fallback: derive from internal registry if present,
                # otherwise just return get_service if any.
                try:
                    svcs = list(pm._services.get(name, []))  # type: ignore[attr-defined]
                except Exception:
                    svcs = []
                try:
                    s = pm.get_service(name)
                    if s and s not in svcs: svcs.append(s)
                except Exception:
                    pass
                return svcs
            pm.get_services = _get_services  # type: ignore[attr-defined]
        if not hasattr(pm, "run_systems"):
            pm.run_systems = lambda engine, dt: None  # type: ignore[attr-defined]
        return pm

def select_service(pm, name, prefer_ids=None, include=None, exclude=None):
    # Build candidate list
    cands = []
    try:
        s = pm.get_service(name)
        if s: cands.append(s)
    except Exception:
        pass
    try:
        for s in pm.get_services(name):
            if s and s not in cands: cands.append(s)
    except Exception:
        pass

    # Filter by include/exclude using service "origin id"
    def origin_id(s):
        try:
            return getattr(s, "__origin__", None) or getattr(s, "__module__", "") or ""
        except Exception:
            return ""
    filtered = []
    for s in cands:
        oid = origin_id(s)
        if include and oid not in include: 
            continue
        if exclude and oid in exclude:
            continue
        filtered.append((oid, s))

    if not filtered:
        return None

    # Prefer specific plugin ids if requested
    for pid in (prefer_ids or []):
        for oid, s in filtered:
            if pid and pid in oid:
                return s

    return filtered[0][1]

def main():
    # Preflight checks
    try:
        from sagecore.core.preflight import run_preflight
        pf = run_preflight(prefer="auto")
        print("[Preflight]", pf)
    except Exception as e:
        print("[Preflight] skipped:", e)

    parser = argparse.ArgumentParser('Custom Engine')
    parser.add_argument('--width', type=int, default=1280)
    parser.add_argument('--height', type=int, default=720)
    parser.add_argument('--title', type=str, default='Custom Engine')
    parser.add_argument('--srgb', action='store_true')
    parser.add_argument('--backend', type=str, choices=['auto','sdl2','glfw','headless'], default='auto')
    parser.add_argument('--max-frames', type=int, default=0, help='>0 to stop after N frames (useful for CI)')
    parser.add_argument('--profile', type=str, choices=['auto','2d','3d','all'], default='auto')
    parser.add_argument('--enable', type=str, default='', help='comma-separated plugin ids to include')
    parser.add_argument('--disable', type=str, default='', help='comma-separated plugin ids to exclude')
    parser.add_argument('--render', type=str, choices=['auto','2d','3d'], default='auto')
    args = parser.parse_args()

    plugins_dir = os.path.join(os.getcwd(), 'plugins')
    cfg = PluginLoadConfig(
        profile=args.profile,
        include=[p for p in args.enable.split(',') if p],
        exclude=[p for p in args.disable.split(',') if p],
    )
    pm = make_pm(plugins_dir, cfg)
    pm.load_plugins()

    def make_window():
        if args.backend == 'headless':
            from sagecore.platform.dummy_window import DummyWindow
            return DummyWindow(args.width, args.height, max_frames=args.max_frames or 600)
        if args.backend in ('auto','sdl2'):
            try:
                from sagecore.platform.sdl2_window import SDL2Window
                return SDL2Window(args.width, args.height, title=args.title, srgb=args.srgb)
            except Exception as e:
                print('[SDL2] Unavailable or missing DLLs:', e)
        try:
            from sagecore.platform.glfw_window import GLFWWindow
            return GLFWWindow(args.width, args.height, title=args.title, srgb=args.srgb)
        except Exception:
            from sagecore.platform.dummy_window import DummyWindow
            return DummyWindow(args.width, args.height, max_frames=args.max_frames or 600)

    window = make_window()

    include_ids = set([p for p in args.enable.split(',') if p])
    exclude_ids = set([p for p in args.disable.split(',') if p])

    # Renderer
    prefer = None
    if args.render != 'auto':
        prefer = ['sage2d'] if args.render == '2d' else ['sage3d']
    rf = None if args.backend == 'headless' else select_service(pm, 'renderer_factory', prefer_ids=prefer, include=include_ids or None, exclude=exclude_ids or None)

    if rf is None:
        class NullRenderer:
            def set_wireframe(self, on): pass
            def set_shade(self, mode): pass
            def render(self, scene, camera, window, dt): pass
        renderer = NullRenderer()
    else:
        renderer = rf()

    # World & Camera
    wf = select_service(pm, 'world_factory', include=include_ids or None, exclude=exclude_ids or None)
    cf = select_service(pm, 'camera_factory', include=include_ids or None, exclude=exclude_ids or None)
    if wf is None:
        from sagecore.core.world_null import NullWorld
        scene = NullWorld()
    else:
        scene = wf()
    if cf is None:
        from sagecore.core.camera_null import NullCamera
        camera = NullCamera()
    else:
        camera = cf()

    eng = Engine(window, renderer, scene, camera)
    eng.plugins = pm
    eng.run()

if __name__ == '__main__':
    main()
