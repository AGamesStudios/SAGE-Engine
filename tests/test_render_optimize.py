from sage_engine.render.optimizer.chunks import ChunkGrid
from sage_engine.render.optimizer.culling import is_visible, cull
from sage_engine.gfx.batch import batch_rect
from sage_engine.render.budget import FrameBudget
from sage_engine.render.scheduler import PredictiveScheduler
import time


def test_chunk_activation():
    grid = ChunkGrid(chunk_size=10)
    obj = object()
    grid.add(obj, (5, 5, 15, 15))
    assert obj in grid.active((0, 0, 10, 10))
    assert obj not in grid.active((20, 20, 30, 30))


def test_culling():
    objects = [("a", (0, 0, 5, 5)), ("b", (15, 15, 25, 25))]
    vis = cull(objects, (10, 10, 20, 20))
    assert vis == ["b"]
    assert not is_visible((0, 0, 1, 1), (5, 5, 10, 10))


def test_batch_rect():
    batches = batch_rect([(0, 0, 4, 4, "red"), (10, 10, 4, 4, "red")])
    assert len(batches) == 1
    ((w, h, color), items) = next(iter(batches.items()))
    assert len(items) == 2 and w == 4 and color == "red"


def test_frame_budget():
    fb = FrameBudget(1)
    fb.start_frame()
    time.sleep(0.002)
    assert not fb.within_budget()


def test_predictive_scheduler():
    sched = PredictiveScheduler()
    for t in [0.02, 0.02, 0.02]:
        sched.record(t)
    assert sched.should_defer(10)
