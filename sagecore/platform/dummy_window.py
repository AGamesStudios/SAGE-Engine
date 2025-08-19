
class DummyWindow:
    def __init__(self, width=1280, height=720, title="Headless", srgb=False, max_frames=300):
        self.width=int(width); self.height=int(height); self._frames=0; self._max=int(max_frames)
        self._pressed=set(); self._keys=set()
    def poll(self): self._pressed.clear(); self._frames += 1
    def swap(self): pass
    def should_close(self): return self._frames >= self._max
    def is_key_down(self, key): return False
    def was_key_pressed(self, key): return False
    def get_cursor_pos(self): return (self.width*0.5, self.height*0.5)
