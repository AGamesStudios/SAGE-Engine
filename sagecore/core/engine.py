
import time, os
try:
    import glfw
except Exception:
    class _GlfwStub:
        KEY_F5=294; KEY_F6=295; KEY_F7=296; KEY_F8=297
        KEY_ESCAPE=256; KEY_LEFT_CONTROL=341
        KEY_W=87; KEY_A=65; KEY_S=83; KEY_D=68; KEY_SPACE=32; KEY_LEFT_SHIFT=340
    glfw=_GlfwStub()
from ..events import EventBus
from .plugins import PluginManager
from .roles_runtime import BehaviorSystem
from .commands import CommandRegistry
from .scene_io import save_scene, load_scene

class Engine:
    def __init__(self, window, renderer, scene, camera):
        self.window=window; self.renderer=renderer; self.scene=scene; self.camera=camera; self.running=True
        self.plugins = PluginManager(os.path.join(os.getcwd(), 'plugins')); self.plugins.load_plugins()
        self.behaviors = BehaviorSystem.get(); self.commands = CommandRegistry.get(); self.events = EventBus.get()
        self.commands.set_default_engine(self)
        self.plugins.register_service('engine', self)
        if False and not getattr(self.scene,'objects',[]): 
            self.scene.objects.append({'role':'camera','params':{'position':[0,1.2,-4],'rotation_euler':[0,0,0]}})
            self.scene.objects.append({'role':'mesh','mesh':{'mesh':'cube','size':1.0}, 'params':{'position':[0,0.5,2]}})
        self._last = time.perf_counter()
    def run(self):
        try: self.events.emit('engine.start', engine=self)
        except Exception: pass
        while self.running and (not hasattr(self.window,'should_close') or not self.window.should_close()):
            now=time.perf_counter(); dt=max(1e-6, now-self._last); self._last=now
            if hasattr(self.window,'poll'): self.window.poll()
            if hasattr(self.window,'was_key_pressed'):
                if self.window.was_key_pressed(glfw.KEY_ESCAPE): self.running=False
                if self.window.was_key_pressed(glfw.KEY_F5): self.commands.execute('toggle_wireframe')
                if self.window.was_key_pressed(glfw.KEY_F6): self.commands.execute('toggle_shade')
                if self.window.was_key_pressed(glfw.KEY_F7):
                    try: os.makedirs('scenes',exist_ok=True); save_scene(self.scene, os.path.join('scenes','auto.json'), self.plugins)
                    except Exception: pass
                if self.window.was_key_pressed(glfw.KEY_F8):
                    try: load_scene(self.scene, os.path.join('scenes','auto.json'), self.plugins)
                    except Exception: pass
            try: self.camera.handle_input(self.window, dt)
            except Exception: pass
            try: self.behaviors.update_all(getattr(self.scene,'objects',[]), dt, self)
            except Exception: pass
            try:
                self.plugins.run_renderer_hooks('pre', self.renderer, self.scene, self.camera, dt)
                self.renderer.render(self.scene, self.camera, self.window, dt)
                self.plugins.run_renderer_hooks('post', self.renderer, self.scene, self.camera, dt)
            except Exception: pass
            if hasattr(self.window,'swap'): self.window.swap()
        try: self.events.emit('engine.shutdown', engine=self)
        except Exception: pass
