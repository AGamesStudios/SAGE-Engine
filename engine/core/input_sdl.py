import sdl2

class SDLInput:
    """Keyboard and mouse input using PySDL2."""
    __slots__ = ("_keys", "_buttons")

    def __init__(self):
        sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO)
        self._keys = set()
        self._buttons = set()

    def poll(self):
        event = sdl2.SDL_Event()
        while sdl2.SDL_PollEvent(event):
            t = event.type
            if t == sdl2.SDL_KEYDOWN:
                self._keys.add(event.key.keysym.sym)
            elif t == sdl2.SDL_KEYUP:
                self._keys.discard(event.key.keysym.sym)
            elif t == sdl2.SDL_MOUSEBUTTONDOWN:
                self._buttons.add(event.button.button)
            elif t == sdl2.SDL_MOUSEBUTTONUP:
                self._buttons.discard(event.button.button)

    def is_key_down(self, key):
        return key in self._keys

    def is_button_down(self, button):
        return button in self._buttons

    def shutdown(self):
        sdl2.SDL_Quit()

