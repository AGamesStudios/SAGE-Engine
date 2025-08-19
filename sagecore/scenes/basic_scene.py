
class BasicScene:
    def __init__(self): self.objects=[]
    def get_drawlist(self):
        for o in self.objects:
            mesh=o.get('mesh', {'mesh':'cube','size':1.0})
            yield (o, mesh)

    def add(self, role, **params):
        o = {'role': str(role), 'params': dict(params or {})}
        # default mesh: unit cube; size/scale can be overridden via params
        if 'mesh' not in o:
            o['mesh'] = {'mesh': 'cube', 'size': float(params.get('size', params.get('scale', [1,1,1])[0] if isinstance(params.get('scale'), (list,tuple)) else 1.0))}
        self.objects.append(o)
        return o
