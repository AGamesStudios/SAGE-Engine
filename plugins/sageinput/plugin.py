
def register(api):
    def input_system(engine=None, dt:float=0.0):
        w = getattr(engine, "window", None)
        if not w: return
        try:
            if w.was_key_pressed(256): engine.running=False  # ESC
            if w.was_key_pressed(294): api.invoke_command('toggle_wireframe', engine=engine)  # F5
            if w.was_key_pressed(295): api.invoke_command('toggle_shade', engine=engine)      # F6
        except Exception:
            pass
    api.register_system("input.hotkeys", input_system, order=10)
