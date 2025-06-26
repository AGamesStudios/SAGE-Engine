import sdl2


class SDL2Input:
    """Keyboard and mouse input using SDL2 events."""

    __slots__ = ("renderer", "_keys", "_buttons")

    def __init__(self, renderer):
        self.renderer = renderer
        self._keys: set[int] = set()
        self._buttons: set[int] = set()

    def poll(self) -> None:
        event = sdl2.SDL_Event()
        while sdl2.SDL_PollEvent(event):
            etype = event.type
            if etype == sdl2.SDL_QUIT:
                self.renderer._should_close = True
            elif etype == sdl2.SDL_KEYDOWN:
                self._keys.add(event.key.keysym.sym)
            elif etype == sdl2.SDL_KEYUP:
                self._keys.discard(event.key.keysym.sym)
            elif etype == sdl2.SDL_MOUSEBUTTONDOWN:
                self._buttons.add(event.button.button)
            elif etype == sdl2.SDL_MOUSEBUTTONUP:
                self._buttons.discard(event.button.button)
            elif etype == sdl2.SDL_WINDOWEVENT:
                if event.window.event == sdl2.SDL_WINDOWEVENT_CLOSE:
                    self.renderer._should_close = True
                elif event.window.event == sdl2.SDL_WINDOWEVENT_RESIZED:
                    self.renderer.set_window_size(event.window.data1, event.window.data2)

    def is_key_down(self, key: int) -> bool:
        return key in self._keys

    def is_button_down(self, button: int) -> bool:
        return button in self._buttons

    def shutdown(self) -> None:
        pass
