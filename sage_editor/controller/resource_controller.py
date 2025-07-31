class ResourceController:
    """Manage project assets."""

    def __init__(self) -> None:
        self.resources = []

    def add_resource(self, path: str) -> None:
        self.resources.append(path)

    def list_resources(self) -> list[str]:
        return list(self.resources)
