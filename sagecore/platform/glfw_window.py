
import glfw
class GLFWWindow:
    def __init__(self, width=1280, height=720, title="SAGE Engine", srgb=False):
        if not glfw.init(): raise RuntimeError("GLFW init failed")
        glfw.window_hint(glfw.SRGB_CAPABLE, glfw.TRUE if srgb else glfw.FALSE)
        self._win = glfw.create_window(int(width), int(height), title, None, None)
        if not self._win:
            glfw.terminate(); raise RuntimeError("GLFW create_window failed")
        glfw.make_context_current(self._win)
        self.width, self.height = int(width), int(height)
        self._keys=set(); self._pressed=set(); self.mouse_captured=False
        glfw.set_key_callback(self._win, self._on_key)
        glfw.set_cursor_pos(self._win, self.width*0.5, self.height*0.5)
    def _on_key(self, win, key, sc, action, mods):
        if action == glfw.PRESS: self._keys.add(key); self._pressed.add(key)
        elif action == glfw.RELEASE:
            if key in self._keys: self._keys.remove(key)
    def poll(self): self._pressed.clear(); glfw.poll_events()
    def swap(self): glfw.swap_buffers(self._win)
    def should_close(self): return bool(glfw.window_should_close(self._win))
    def is_key_down(self, key): return key in self._keys
    def was_key_pressed(self, key): return key in self._pressed
    def get_cursor_pos(self): return glfw.get_cursor_pos(self._win)
    def set_title(self, title:str): glfw.set_window_title(self._win, str(title))
