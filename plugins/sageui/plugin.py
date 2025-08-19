
def register(api):
    state = {"acc":0.0, "frames":0}
    def fps_system(engine=None, dt:float=0.0):
        w = getattr(engine, "window", None)
        if not w or not hasattr(w, "set_title"): return
        state["acc"] += dt; state["frames"] += 1
        if state["acc"] >= 1.0:
            fps = state["frames"]/max(1e-6, state["acc"])
            try: w.set_title(f"Custom Engine  |  FPS: {fps:.1f}")
            except Exception: pass
            state["acc"] = 0.0; state["frames"] = 0
    api.register_system("ui.fps_title", fps_system, order=90)
