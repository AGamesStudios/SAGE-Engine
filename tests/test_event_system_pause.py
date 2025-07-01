from engine.logic import Event, EventSystem

class DummyAct:
    def __init__(self):
        self.called = 0
    def execute(self, engine, scene, dt):
        self.called += 1


def test_pause_resume():
    act = DummyAct()
    evt = Event([], [act], name='e')
    es = EventSystem()
    es.add_event(evt)
    es.update(None, None, 0.1)
    assert act.called == 1
    es.pause()
    es.update(None, None, 0.1)
    assert act.called == 1
    es.resume()
    es.update(None, None, 0.1)
    assert act.called == 2


def test_update_group():
    act1 = DummyAct()
    act2 = DummyAct()
    e1 = Event([], [act1], name='e1', groups=['a'])
    e2 = Event([], [act2], name='e2', groups=['b'])
    es = EventSystem()
    es.add_event(e1)
    es.add_event(e2)
    es.update_group('a', None, None, 0.1)
    assert act1.called == 1
    assert act2.called == 0
