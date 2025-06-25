import pygame


class PygameInput:
    """Keyboard and mouse input using pygame."""

    __slots__ = ("_pygame", "_keys", "_buttons", "renderer")

    def __init__(self, renderer):
        self._pygame = pygame
        self.renderer = renderer
        self._keys = set()
        self._buttons = set()

    def poll(self):
        for event in self._pygame.event.get():
            if event.type == self._pygame.KEYDOWN:
                self._keys.add(event.key)
            elif event.type == self._pygame.KEYUP:
                self._keys.discard(event.key)
            elif event.type == self._pygame.MOUSEBUTTONDOWN:
                self._buttons.add(event.button)
            elif event.type == self._pygame.MOUSEBUTTONUP:
                self._buttons.discard(event.button)
            elif event.type == self._pygame.VIDEORESIZE and self.renderer.window:
                self.renderer.set_window_size(event.w, event.h)
            elif event.type == self._pygame.QUIT:
                self.renderer._should_close = True

    def is_key_down(self, key):
        return key in self._keys

    def is_button_down(self, button):
        return button in self._buttons

    def shutdown(self):
        pass
