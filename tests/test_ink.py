from sage_engine.ink import InkEmitter


def test_emitter_cap():
    emitter = InkEmitter(rate=50, life_time=0.5)
    for _ in range(60):
        emitter.emit(0.02)
        emitter.update(0.02)
    assert len(emitter.particles) <= int(50 * 0.5) + 1
