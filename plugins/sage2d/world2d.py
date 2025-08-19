
class BasicWorld2D:
    def __init__(self): self.objects=[]
    def add(self, role, **params):
        o={'role':str(role),'params':dict(params or {})}; self.objects.append(o); return o
    def get_drawlist(self):
        for o in self.objects: yield (o, o.get('params',{}))
