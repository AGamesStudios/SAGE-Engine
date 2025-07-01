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

from engine.inputs import InputManager, get_input


def test_action_binding():
    backend = DummyBackend()
    m = InputManager(backend)
    m.bind_action('jump', key=1)
    backend.keys.add(1)
    assert m.is_action_down('jump')
    m.unbind_action('jump')
    assert not m.is_action_down('jump')


def test_axis_and_callbacks():
    backend = DummyBackend()
    m = InputManager(backend)
    m.bind_axis('move', positive=1, negative=2)
    events = []
    m.bind_action('fire', key=3)
    m.on_press('fire', lambda: events.append('press'))
    m.on_release('fire', lambda: events.append('release'))
    backend.keys.update({1, 3})
    m.poll()
    assert m.get_axis('move') == 1
    backend.keys.discard(1)
    backend.keys.add(2)
    m.poll()
    assert m.get_axis('move') == -1
    backend.keys.discard(3)
    m.poll()
    assert events == ['press', 'release']


def test_null_backend_registry():
    cls = get_input('null')
    assert cls is not None
    backend = cls()
    m = InputManager(backend)
    m.poll()
    assert not m.is_action_down('any')
    m.shutdown()
