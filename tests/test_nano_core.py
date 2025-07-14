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

