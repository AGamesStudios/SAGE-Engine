
import numpy as np
class NullCamera:
    def get_view(self):
        return np.eye(4, dtype=np.float32)
    def get_proj(self, aspect: float = 16/9):
        return np.eye(4, dtype=np.float32)
    def handle_input(self, window, dt: float): pass
