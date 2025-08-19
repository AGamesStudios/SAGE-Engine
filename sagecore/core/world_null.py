
class NullWorld:
    """Minimal world placeholder; renderers may ignore it."""
    def __init__(self): pass
    def get_drawlist(self):
        return []
