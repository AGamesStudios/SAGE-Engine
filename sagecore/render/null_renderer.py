
class NullRenderer:
    def __init__(self): self.wire=False
    def set_wireframe(self, on:bool): self.wire=bool(on)
    def set_shade(self, mode:str): pass
    def render(self, scene, camera, window, dt:float): pass
