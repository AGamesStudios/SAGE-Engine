from . import InputBackend, register_input

class NullInput(InputBackend):
    """Dummy backend that reports no input."""

    def __init__(self, widget=None):
        pass

    def poll(self) -> None:
        pass

    def is_key_down(self, key: int) -> bool:
        return False

    def is_button_down(self, button: int) -> bool:
        return False

    def get_axis_value(self, axis_id: int) -> float:
        return 0.0

    def shutdown(self) -> None:
        pass

register_input("null", NullInput)
