
class ScriptBridge:
    """Abstract bridge. Concrete bridges must implement load() and call()."""
    lang = "abstract"
    def __init__(self, api=None):
        self.api = api
    def load(self, entry, **kw):
        raise NotImplementedError
    def call(self, fn, *args, **kwargs):
        raise NotImplementedError
    def expose(self, name, obj):
        """Optionally expose engine API into the script environment."""
        setattr(self, name, obj)
