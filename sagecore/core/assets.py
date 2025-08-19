
class AssetRegistry:
    _inst = None
    def __init__(self):
        self._meshes = {}; self._textures = {}
    @classmethod
    def get(cls):
        if not cls._inst: cls._inst = cls()
        return cls._inst
    def create_mesh_from_cpu(self, name, data): self._meshes[name]=data; return data
    def get_mesh(self, name): return self._meshes.get(name)
    def create_texture_from_cpu(self, name, data): self._textures[name]=data; return data
    def get_texture(self, name): return self._textures.get(name)
