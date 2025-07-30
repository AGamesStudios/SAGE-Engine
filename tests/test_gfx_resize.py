import os
from sage_engine import gfx, events


def test_resize_event_triggers_realloc():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    gfx.init(10, 10)
    events.emit('window_resized', 20, 15)
    events.dispatcher.flush()
    assert gfx._runtime.width == 20 and gfx._runtime.height == 15
    gfx.shutdown()
