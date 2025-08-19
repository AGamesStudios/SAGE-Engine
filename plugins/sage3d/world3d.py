
class BasicWorld3D:
    def __init__(self):
        self.objects = []
    def add(self, role, **params):
        o={'role':str(role),'params':dict(params or {})}
        if 'mesh' not in o and role=='mesh':
            o['mesh']={'mesh':'cube','size':float(params.get('size',1.0))}
        self.objects.append(o); return o
    def get_drawlist(self):
        for o in self.objects:
            mesh=o.get('mesh', {'mesh':'cube','size':1.0})
            yield (o, mesh)
