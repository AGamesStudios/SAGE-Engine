class BlueprintController:
    """Manage blueprint data."""

    def __init__(self) -> None:
        self.blueprints = []

    def add_blueprint(self, name: str) -> None:
        self.blueprints.append(name)

    def list_blueprints(self) -> list[str]:
        return list(self.blueprints)
