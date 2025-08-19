
from __future__ import annotations
import importlib.util
from importlib.metadata import entry_points
import sys, os, json, types
from .compat import CORE_VERSION, Capabilities, satisfies
try:
    from .object import RoleSystem, CategorySystem, ComponentSystem, TagSystem  # type: ignore
except Exception:
    RoleSystem = CategorySystem = ComponentSystem = TagSystem = object

def _manifest_requires_ok(manifest: dict, api) -> bool:
    try:
        req = (manifest or {}).get('requires', {}) or {}
        core_req = req.get('core')
        if core_req and not satisfies(api.core_version, core_req):
            return False
        feats = req.get('features', {}) or {}
        for k, expr in feats.items():
            if not api.supports(k, expr):
                return False
        return True
    except Exception:
        return True

class PluginAPI:
    def __init__(self, manager: 'PluginManager' | None = None):
        self._manager = manager; self._services = {}
        self._systems = []
        self._meta = {}
        from .roles_runtime import BehaviorSystem
        from .commands import CommandRegistry
        self.behaviors = BehaviorSystem.get(); self.commands = CommandRegistry.get()
        self._caps = Capabilities(); self.core_version = CORE_VERSION
        try:
            self.roles      = RoleSystem if isinstance(RoleSystem, type) else RoleSystem()
            self.categories = CategorySystem if isinstance(CategorySystem, type) else CategorySystem()
            self.components = ComponentSystem if isinstance(ComponentSystem, type) else ComponentSystem()
            self.tags       = TagSystem if isinstance(TagSystem, type) else TagSystem()
        except Exception:
            self.roles = self.categories = self.components = self.tags = object
    def version(self): return self.core_version
    def features(self): return self._caps.all()
    def supports(self, feature, requirement=None): 
        try: return self._caps.supports(feature, requirement)
        except Exception: return True
    def require(self, **reqs):
        core_req = reqs.pop('core', None)
        if core_req and not satisfies(self.core_version, core_req):
            raise RuntimeError(f'core {self.core_version} does not satisfy {core_req}')
        for feat, expr in reqs.items():
            if not self.supports(feat, expr):
                have = self._caps.get(feat)
                raise RuntimeError(f'feature {feat} not satisfied: need {expr}, have {have!r}')
    def register_role(self, name, **params):
        if hasattr(self.roles, 'define'):
            try: return self.roles.define(name, **params)
            except Exception: return None
    def register_behavior(self, role_name, behavior):
        try: self.behaviors.register_behavior(role_name, behavior)
        except Exception: pass
    def register_command(self, name, fn, safe=True, trust='untrusted'):
        origin = getattr(self, '_current_plugin', 'core')
        try: self.commands.register(name, fn, safe=safe, origin=origin, trust=trust)
        except Exception: pass
    def register_scene_hooks(self, on_save=None, on_load=None):
        m = getattr(self, "_manager", None); 
        if not m: return
        if on_save and callable(on_save): m._scene_hooks.setdefault('on_scene_save', []).append(on_save)
        if on_load and callable(on_load): m._scene_hooks.setdefault('on_scene_load', []).append(on_load)
    role = register_role; add_role = register_role; add_behavior = register_behavior; command = register_command
    def register_renderer_hook(self, hook_name, fn):
        m = getattr(self, "_manager", None); 
        if not m: return
        m._renderer_hooks.setdefault(hook_name, []).append(fn)
    def register_ui_component(self, comp_id, factory):
        m = getattr(self, "_manager", None); 
        if not m: return
        m._ui_components[comp_id] = factory
    def asset_register(self, kind, name, data):
        try:
            from .assets import AssetRegistry
            reg = AssetRegistry.get()
            k = (kind or '').lower()
            if k == 'mesh' and hasattr(reg, 'create_mesh_from_cpu'): return reg.create_mesh_from_cpu(name, data)
            if k == 'texture' and hasattr(reg, 'create_texture_from_cpu'): return reg.create_texture_from_cpu(name, data)
            if hasattr(reg, 'register'): return reg.register(name, data)
        except Exception: pass
        return None
    def provide_service(self, name, obj):
        m = getattr(self, '_manager', None)
        if m: m.register_service(name, obj)
    def get_service(self, name):
        if name in self._services: return self._services[name]
        m = getattr(self, "_manager", None)
        if not m: return None
        return m._services.get(name)
    def settings(self, namespace: str) -> dict:
        m = getattr(self, "_manager", None)
        if not m: return {}
        return m._settings(namespace)
    def register_settings_schema(self, namespace: str, version: int, migrate_fn=None):
        m = getattr(self, "_manager", None)
        if not m: return
        m._settings_schema[namespace] = (int(version), migrate_fn if callable(migrate_fn) else None)
    @property
    def events(self):
        try:
            from ..events import EventBus
            return EventBus.get()
        except Exception:
            return types.SimpleNamespace(emit=lambda *a, **k: None, on=lambda *a, **k: None)
    @property
    def assets(self):
        try:
            from .assets import AssetRegistry
            return AssetRegistry.get()
        except Exception: return None


class PluginLoadConfig:
    def __init__(self, profile:str='auto', include=None, exclude=None):
        self.profile = profile  # 'auto','2d','3d','all'
        self.include = set(include or [])
        self.exclude = set(exclude or [])

class PluginManager:
    def __init__(self, plugins_dir: str, allow_entry_points: bool = False):
        self.plugins_dir = plugins_dir; self.allow_entry_points = allow_entry_points
        self.loaded = []; self.failed = []
        self._services = {}
        self._systems = []
        self._meta = {}; self._renderer_hooks={'pre':[],'post':[]}
        self._ui_components = {}; self._scene_hooks = {}
        self._settings_ns = {}; self._settings_schema = {}
    def _add_archives_to_syspath(self):
        if not os.path.isdir(self.plugins_dir): return
        for f in os.listdir(self.plugins_dir):
            path = os.path.join(self.plugins_dir, f)
            if os.path.isfile(path) and (f.endswith('.zip') or f.endswith('.whl')):
                if path not in sys.path: sys.path.insert(0, path)
    def _load_folder_plugins(self, api: PluginAPI):
        if not os.path.isdir(self.plugins_dir): return
        for entry in os.listdir(self.plugins_dir):
            p = os.path.join(self.plugins_dir, entry)
            if not os.path.isdir(p): continue
            manifest = {}; cfg = os.path.join(p, 'plugin.json')
            if os.path.isfile(cfg):
                try:
                    with open(cfg, 'r', encoding='utf-8') as fh: manifest = json.load(fh) or {}
                    modname = manifest.get('module', 'plugin')
                except Exception: modname = 'plugin'
                modpath = os.path.join(p, modname + '.py')
            else:
                modpath = os.path.join(p, 'plugin.py')
            if not os.path.isfile(modpath): continue
            if not _manifest_requires_ok(manifest, api):
                self.failed.append((entry, 'requirements not satisfied')); continue
            name = f'sage_plugin_{entry}'
            try:
                spec = importlib.util.spec_from_file_location(name, modpath, submodule_search_locations=[os.path.dirname(modpath)])
                mod = importlib.util.module_from_spec(spec)  # type: ignore
                import sys as _sys; _sys.modules[name] = mod
                assert spec and spec.loader
                spec.loader.exec_module(mod)  # type: ignore
                reg = getattr(mod, 'register', None) or getattr(mod, 'register_v1', None) or getattr(mod, 'register_v0', None)
                if callable(reg):
                    setattr(api, '_current_plugin', entry); reg(api); setattr(api, '_current_plugin', None)
                    self.loaded.append(entry)
            except Exception as e:
                self.failed.append((entry, str(e))); continue
    def _load_entry_points(self, api: PluginAPI):
        try:
            eps = entry_points()
            group = eps.select(group='sage3d.plugins') if hasattr(eps, 'select') else eps.get('sage3d.plugins', [])
            for ep in group:
                try:
                    obj = ep.load()
                    if callable(obj): obj(api); self.loaded.append(f'ep:{ep.name}')
                    elif hasattr(obj, 'register'): obj.register(api); self.loaded.append(f'ep:{ep.name}')
                except Exception as e:
                    self.failed.append((f'ep:{ep.name}', str(e)))
        except Exception: pass
    def load_plugins(self) -> PluginAPI:
        api = PluginAPI(self)
        self._add_archives_to_syspath(); self._load_folder_plugins(api)
        if self.allow_entry_points: self._load_entry_points(api)
        return api
    def register_service(self, name, obj): self._services[name] = obj
    def get_service(self, name): return self._services.get(name)
    def run_renderer_hooks(self, when, renderer, scene, camera, dt: float):
        for fn in self._renderer_hooks.get(when, []):
            try: fn(renderer=renderer, scene=scene, camera=camera, dt=dt)
            except Exception: pass
    def _settings_flush(self, ns: str, data: dict):
        try:
            os.makedirs('settings', exist_ok=True)
            f = os.path.join('settings', f'{ns}.json')
            with open(f,'w',encoding='utf-8') as fh: json.dump(data or {}, fh, ensure_ascii=False, indent=2)
        except Exception: pass
    def _settings(self, ns: str) -> dict:
        if ns not in self._settings_ns:
            self._settings_ns[ns] = {}
            try:
                os.makedirs('settings', exist_ok=True)
                f = os.path.join('settings', f'{ns}.json')
                if os.path.isfile(f):
                    with open(f,'r',encoding='utf-8') as fh:
                        data = json.load(fh) or {}
                        if isinstance(data, dict): self._settings_ns[ns]=data
                ver, mig = self._settings_schema.get(ns, (None, None))
                if ver is not None:
                    cur = int(self._settings_ns[ns].get('_schema_version', 0))
                    if cur < int(ver):
                        try:
                            if mig:
                                new_data = mig(self._settings_ns[ns], cur, int(ver))
                                if isinstance(new_data, dict): self._settings_ns[ns] = new_data
                            self._settings_ns[ns]['_schema_version'] = int(ver)
                            self._settings_flush(ns, self._settings_ns[ns])
                        except Exception: pass
            except Exception: pass
        parent = self
        class _Proxy(dict):
            def __setitem__(self, k, v): super().__setitem__(k,v); parent._settings_flush(ns, self)
            def update(self, *a, **kw): super().update(*a, **kw); parent._settings_flush(ns, self)
            def setdefault(self, k, d=None):
                if k not in self:
                    super().setdefault(k, d); parent._settings_flush(ns, self); return self[k]
                return super().setdefault(k, d)
        return _Proxy(self._settings_ns[ns])
    def install(self, path: str, activate: bool = True) -> str:
        try:
            os.makedirs(self.plugins_dir, exist_ok=True)
            base = os.path.basename(path); dst = os.path.join(self.plugins_dir, base)
            if os.path.abspath(path) != os.path.abspath(dst):
                import shutil as _sh
                if os.path.isdir(path):
                    if os.path.exists(dst): _sh.rmtree(dst)
                    _sh.copytree(path, dst)
                else: _sh.copy2(path, dst)
            if activate:
                try:
                    if base.endswith(('.zip','.whl')) and dst not in sys.path: sys.path.insert(0, dst)
                    self.load_plugins()
                except Exception as e: return f"installed but activation failed: {e}"
            return f"installed: {base}"
        except Exception as e: return f"error: {e}"
    def reload(self, name: str | None = None) -> str:
        try:
            reloaded = []
            if name:
                to_del = []
                for mod in list(sys.modules.keys()):
                    if mod == name or mod.endswith(name) or mod.startswith(f"sage_plugin_{name}"): to_del.append(mod)
                for m in to_del:
                    try: del sys.modules[m]
                    except Exception: pass
                self.load_plugins(); reloaded.append(name)
            else:
                for m in list(sys.modules.keys()):
                    if m.startswith("sage_plugin_"):
                        try: del sys.modules[m]
                        except Exception: pass
                self.loaded.clear(); self.failed.clear(); self.load_plugins(); reloaded = list(self.loaded)
            return f"reloaded: {', '.join(reloaded) if reloaded else '(none)'}"
        except Exception as e: return f"error: {e}"
    def rescan(self) -> str:
        try:
            self.loaded.clear(); self.failed.clear(); self.load_plugins()
            return f"rescan: loaded={len(self.loaded)} failed={len(self.failed)}"
        except Exception as e: return f"error: {e}"
    def uninstall(self, name: str) -> str:
        try:
            target_mods = [m for m in list(sys.modules.keys()) if m.endswith(name) or m.startswith(f"sage_plugin_{name}")]
            for m in target_mods:
                mod = sys.modules.get(m)
                if mod and hasattr(mod, 'deactivate') and callable(mod.deactivate):
                    try: mod.deactivate()
                    except Exception: pass
                try: del sys.modules[m]
                except Exception: pass
            p = os.path.join(self.plugins_dir, name)
            if os.path.isdir(p):
                import shutil; shutil.rmtree(p, ignore_errors=True)
            else:
                if os.path.isdir(self.plugins_dir):
                    for f in list(os.listdir(self.plugins_dir)):
                        if f == name or f.startswith(name):
                            try: os.remove(os.path.join(self.plugins_dir, f))
                            except Exception: pass
            try: self.loaded.remove(name)
            except Exception: pass
            return f'uninstalled: {name}'
        except Exception as e: return f'error: {e}'


def get_services(self, name: str):
    return list(self._services.get(name, []))


def register_system(self, name: str, fn, order: int = 100):
    """Register a per-frame system. Called each frame as fn(engine=..., dt=...)."""
    self._systems.append((int(order), str(name), fn))

def run_systems(self, engine, dt: float):
    for _, _, fn in sorted(self._systems, key=lambda x: x[0]):
        try: fn(engine=engine, dt=dt)
        except Exception: pass


def _allowed_by_config(self, manifest: dict):
    cfg = getattr(self, 'config', None)
    if not cfg: 
        return True
    pid = str(manifest.get('id') or manifest.get('name'))
    if pid in cfg.exclude: return False
    if cfg.include and pid not in cfg.include: return False
    profs = set((manifest.get('profiles') or []) + (manifest.get('tags') or []))
    if cfg.profile in ('all','auto'): return True
    if cfg.profile == '2d': return bool({'2d','render:2d'} & profs)
    if cfg.profile == '3d': return bool({'3d','render:3d'} & profs)
    return True
