def _load_local(module_filename):
    import importlib.util, os
    here=os.path.dirname(__file__)
    path=os.path.join(here,module_filename)
    name=f"sage_plugin_sage2d_{module_filename.replace('.py','')}"
    spec=importlib.util.spec_from_file_location(name,path)
    mod=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

def register(api):
    try: api.verify_requirements({"core": ">=1.2.0", "render": ">=1.0"})
    except Exception: pass

    def renderer_factory():
        try:
            from .renderer2d import OpenGL2DRenderer
        except Exception:
            import sys, importlib, pathlib
            sys.path.insert(0, str(pathlib.Path(__file__).parent))
            OpenGL2DRenderer = importlib.import_module('renderer2d').OpenGL2DRenderer
        return OpenGL2DRenderer(api)
    api.provide_service('renderer_factory', renderer_factory)
    api.provide_service('world_factory', lambda: _load_local('world2d.py').BasicWorld2D())
    api.provide_service('camera_factory', lambda: _load_local('camera2d.py').Camera2D())

    def _cmd_toggle_wireframe(engine=None, **_):
        try: engine.renderer.toggle_wireframe()
        except Exception: pass
    api.register_command('toggle_wireframe', _cmd_toggle_wireframe, safe=True)

    def on_start(engine=None, **_):
        try:
            if not getattr(engine.scene,'objects',[]):
                engine.scene.add('sprite', position=[0,0,0], rotation_euler=[0,0,0], size=1.0)
        except Exception: pass
    api.events.on('engine.start', on_start)
