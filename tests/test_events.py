from sage_engine import events, profiling


def test_event_emit():
    got = []

    def handler(x):
        got.append(x)

    events.dispatcher.on(1, handler)
    events.dispatcher.emit(1, 42)
    events.dispatcher.flush()
    assert got == [42]


def test_event_overflow():
    events.reset()
    count = 0

    def handler(x):
        nonlocal count
        count += 1

    events.dispatcher.on(1, handler)
    for i in range(events.dispatcher.MAX_PER_FRAME + 10):
        events.dispatcher.emit(1, i)
    events.dispatcher.flush()
    assert count == events.dispatcher.MAX_PER_FRAME
    assert profiling.profile.events_dropped > 0

def test_event_priority_and_history():
    events.reset()
    order = []

    def a():
        order.append('a')

    def b():
        order.append('b')

    events.dispatcher.on(1, lambda: order.append('c'))
    events.dispatcher.emit(1, priority=0)
    events.dispatcher.emit(2, priority=5)
    events.dispatcher.on(2, b)
    events.dispatcher.on(1, a)
    events.dispatcher.flush()
    assert order == ['b', 'c', 'a']
    assert 2 in events.dispatcher.history
