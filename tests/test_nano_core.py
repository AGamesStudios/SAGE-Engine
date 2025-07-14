import threading
import pytest

nano_core = pytest.importorskip("nano_core")


def test_nano_core_functions():
    assert nano_core.merge_chunk_delta(b"a", b"b") == b"ab"
    data = nano_core.alloc_smart_slice(1)
    assert len(data) == 1024


def test_nano_core_thread():
    out = []

    def worker():
        out.append(nano_core.merge_chunk_delta(b"x", b"y"))

    t = threading.Thread(target=worker)
    t.start()
    t.join()
    assert out == [b"xy"]


def test_reset_tree_and_stress():
    import tracemalloc
    import gc

    tracemalloc.start()
    before = tracemalloc.take_snapshot()

    blocks = [nano_core.alloc_smart_slice(1) for _ in range(100)]
    result = b""
    for i in range(10_000):
        result = nano_core.merge_chunk_delta(result, bytes([i % 256]))
    assert len(result) == 10_000

    del blocks
    nano_core.reset_tree()
    gc.collect()

    after = tracemalloc.take_snapshot()
    diff = sum(s.size_diff for s in after.compare_to(before, "filename"))
    tracemalloc.stop()
    assert abs(diff) < 1024 * 200

