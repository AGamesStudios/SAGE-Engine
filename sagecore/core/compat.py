
CORE_VERSION = "1.2.0"
class Capabilities(dict):
    def all(self): return dict(self)
    def get(self, k): return super().get(k, None)
    def supports(self, feature, requirement=None): return True
def satisfies(ver: str, expr: str) -> bool:
    return True
