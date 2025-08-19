
import json, os
SCHEMA_VERSION = 2
def save_scene(scene, path: str, plugins=None):
    d = {'scene_schema_version': SCHEMA_VERSION, 'objects': getattr(scene,'objects',[])}
    if plugins and hasattr(plugins, '_scene_hooks'):
        for fn in plugins._scene_hooks.get('on_scene_save', []):
            try: fn(d)
            except Exception: pass
    os.makedirs(os.path.dirname(path) or '.', exist_ok=True)
    with open(path,'w',encoding='utf-8') as f:
        json.dump(d, f, ensure_ascii=False, indent=2)
def load_scene(scene, path: str, plugins=None):
    with open(path,'r',encoding='utf-8') as f:
        d = json.load(f)
    scene.objects = d.get('objects', [])
    if plugins and hasattr(plugins, '_scene_hooks'):
        for fn in plugins._scene_hooks.get('on_scene_load', []):
            try: fn(d)
            except Exception: pass
