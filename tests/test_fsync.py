from time import perf_counter, sleep
from sage_engine.runtime.fsync import FrameSync


def test_precise_frame_sync():
    fs = FrameSync(target_fps=60)
    durations = []
    for _ in range(3):
        start = perf_counter()
        fs.start_frame()
        fs.end_frame()
        durations.append(perf_counter() - start)
    target = 1 / 60
    for d in durations:
        assert d >= target
        assert abs(d - target) < 0.005


def test_adaptive_mode():
    fs = FrameSync(target_fps=60, mode="adaptive")
    fs.start_frame()
    time_cost = 0.03
    sleep(time_cost)
    fs.end_frame()
    assert fs.target_fps <= 30

