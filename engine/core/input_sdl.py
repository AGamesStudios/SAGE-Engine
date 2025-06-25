import sdl2

class SDLInput:
    """Keyboard and mouse input using PySDL2."""
    __slots__ = ("_keys", "_buttons", "renderer")

    def __init__(self, renderer):
        self.renderer = renderer
        self._keys = set()
        self._buttons = set()

    def poll(self):
        event = sdl2.SDL_Event()
        while sdl2.SDL_PollEvent(event):
            if event.type == sdl2.SDL_QUIT:
                self.renderer._should_close = True
            elif event.type == sdl2.SDL_KEYDOWN:
                self._keys.add(event.key.keysym.sym)
            elif event.type == sdl2.SDL_KEYUP:
                self._keys.discard(event.key.keysym.sym)
            elif event.type == sdl2.SDL_MOUSEBUTTONDOWN:
                self._buttons.add(event.button.button)
            elif event.type == sdl2.SDL_MOUSEBUTTONUP:
                self._buttons.discard(event.button.button)
            elif event.type == sdl2.SDL_WINDOWEVENT and event.window.event == sdl2.SDL_WINDOWEVENT_RESIZED:
                self.renderer.set_window_size(event.window.data1, event.window.data2)

    def is_key_down(self, key):
        return key in self._keys

    def is_button_down(self, button):
        return button in self._buttons

    def shutdown(self):
        pass
