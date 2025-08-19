
def register(api):
    import os, importlib.util
    project_root = os.getcwd()
    scripts_dir = os.path.join(project_root, "Scripts")
    state = {"tick": None}
    def _load():
        path = os.path.join(scripts_dir, "startup.py")
        if not os.path.isfile(path): return
        name = "sage_user_startup"
        spec = importlib.util.spec_from_file_location(name, path)
        mod = importlib.util.module_from_spec(spec)
        try:
            spec.loader.exec_module(mod)
            if hasattr(mod, "setup"): 
                try: mod.setup(api)
                except Exception: pass
            if hasattr(mod, "tick"):
                state["tick"] = getattr(mod, "tick")
        except Exception:
            pass
    _load()
    def script_system(engine=None, dt:float=0.0):
        fn = state.get("tick")
        if fn:
            try: fn(engine, dt)
            except Exception: pass
    api.register_system("scripts.tick", script_system, order=50)
