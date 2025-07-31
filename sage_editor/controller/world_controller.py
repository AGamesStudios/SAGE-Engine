class WorldController:
    """Manage scenes and objects."""

    def __init__(self) -> None:
        self.scenes = []

    def add_scene(self, name: str) -> None:
        self.scenes.append(name)

    def remove_scene(self, name: str) -> None:
        if name in self.scenes:
            self.scenes.remove(name)
