
class Camera2D:
    def get_view(self): 
        import numpy as np; return np.eye(4,dtype=np.float32)
    def get_proj(self, aspect: float = 16/9): 
        import numpy as np; return np.eye(4,dtype=np.float32)
    def handle_input(self, window, dt: float): pass
