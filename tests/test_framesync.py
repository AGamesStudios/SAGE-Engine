import time
from sage_engine.framesync import boot, reset, regulate, get_actual_fps


def test_framesync_basic():
    boot()
    frames = []
    for _ in range(3):
        start = time.perf_counter()
        regulate()
        frames.append(time.perf_counter() - start)
    fps = get_actual_fps()
    reset()
    avg = sum(frames) / len(frames)
    assert abs(avg - 1/60) < 0.02
    assert 40 <= fps <= 70
