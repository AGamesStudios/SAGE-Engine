import types

class DummyBackend:
    def __init__(self):
        self.keys = set()
        self.buttons = set()
    def poll(self):
        pass
    def is_key_down(self, key):
        return key in self.keys
    def is_button_down(self, button):
        return button in self.buttons
    def shutdown(self):
        self.keys.clear()
        self.buttons.clear()

from engine.inputs import InputManager


def test_action_binding():
    backend = DummyBackend()
    m = InputManager(backend)
    m.bind_action('jump', key=1)
    backend.keys.add(1)
    assert m.is_action_down('jump')
    m.unbind_action('jump')
    assert not m.is_action_down('jump')
