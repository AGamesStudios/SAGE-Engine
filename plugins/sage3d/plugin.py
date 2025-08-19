def _load_local(module_filename):
    import importlib.util, os
    here=os.path.dirname(__file__)
    path=os.path.join(here,module_filename)
    name=f"sage_plugin_sage3d_{module_filename.replace('.py','')}"
    spec=importlib.util.spec_from_file_location(name,path)
    mod=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

# SAGE3D plugin entry
def register(api):
    try: api.verify_requirements({"core": ">=1.2.0", "render": ">=1.0"})
    except Exception: pass

    def renderer_factory():
        # Support both package and single-module loading
        try:
            from .opengl_renderer import OpenGLRenderer  # package-style
        except Exception:
            import sys, importlib, pathlib
            sys.path.insert(0, str(pathlib.Path(__file__).parent))
            OpenGLRenderer = importlib.import_module('opengl_renderer').OpenGLRenderer
        return OpenGLRenderer(api)
    api.provide_service('renderer_factory', renderer_factory)
    api.provide_service('world_factory', lambda: _load_local('world3d.py').BasicWorld3D())
    api.provide_service('camera_factory', lambda: _load_local('camera3d.py').Camera3D())

    def camera_factory():
        try:
            from sagecore.camera import Camera
            return Camera()
        except Exception:
            return None
    api.provide_service('camera_factory', camera_factory)

    def _cmd_toggle_wireframe(engine=None, **_):
        try: engine.renderer.set_wireframe(not getattr(engine.renderer,'wire',False))
        except Exception: pass
    def _cmd_toggle_shade(engine=None, **_):
        try: engine.renderer.set_shade('unlit' if getattr(engine.renderer,'shade','lambert')!='unlit' else 'lambert')
        except Exception: pass
    def _cmd_spawn_cube(engine=None, **_):
        try: engine.scene.add('mesh', position=[0,0,0], rotation_euler=[0,0,0], size=1.0)
        except Exception: pass
    def _cmd_reset_camera(engine=None, **_):
        try: c=engine.camera; c.pos[:]=[0,0,-3]; c.rot[:]=[0,0,0]
        except Exception: pass
    def _cmd_toggle_axes(engine=None, **_):
        try:
            if hasattr(engine.renderer,'toggle_axes'): engine.renderer.toggle_axes()
        except Exception: pass

    api.register_command('toggle_wireframe', _cmd_toggle_wireframe, safe=True)
    api.register_command('toggle_shade', _cmd_toggle_shade, safe=True)
    api.register_command('spawn_cube', _cmd_spawn_cube, safe=True)
    api.register_command('reset_camera', _cmd_reset_camera, safe=True)
    api.register_command('toggle_axes', _cmd_toggle_axes, safe=True)

    def on_start(engine=None, **_):
        try:
            if not getattr(engine.scene,'objects',[]):
                engine.scene.add('mesh', position=[0,0,0], rotation_euler=[0,0,0], size=1.0)
        except Exception: pass
    api.events.on('engine.start', on_start)

    def pre_hook(renderer=None, **kw):
        try:
            if hasattr(renderer,'debug_pre'): renderer.debug_pre(**kw)
        except Exception: pass
    api.register_renderer_hook('pre', pre_hook)
