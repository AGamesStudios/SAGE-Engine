class Input:
    __slots__ = ("_glfw", "window", "_keys", "_buttons")

    def __init__(self, window):
        import glfw
        self._glfw = glfw
        self.window = window
        self._keys = set()
        self._buttons = set()
        glfw.set_key_callback(window, self._on_key)
        glfw.set_mouse_button_callback(window, self._on_button)

    def _on_key(self, window, key, scancode, action, mods):
        if action != self._glfw.RELEASE:
            self._keys.add(key)
        else:
            self._keys.discard(key)

    def _on_button(self, window, button, action, mods):
        if action != self._glfw.RELEASE:
            self._buttons.add(button)
        else:
            self._buttons.discard(button)

    def poll(self):
        self._glfw.poll_events()

    def is_key_down(self, key):
        return key in self._keys

    def is_button_down(self, button):
        return button in self._buttons

    def shutdown(self):
        self._glfw.set_key_callback(self.window, None)
        self._glfw.set_mouse_button_callback(self.window, None)
