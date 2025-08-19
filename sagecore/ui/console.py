
class TkConsole:
    def __init__(self, commands, engine=None):
        self.commands = commands; self.engine = engine; self.is_open=False
    def open(self): self.is_open=True; print("[Console] opened")
    def shutdown(self): self.is_open=False
