import mmap
import tempfile
import pytest

from sage_engine.smart_slice import SmartSliceAllocator, SLICE_SIZE


def test_smart_slice_allocator_wrap_and_free():
    size = SLICE_SIZE * 4
    with tempfile.TemporaryFile() as f:
        f.truncate(size)
        mm = mmap.mmap(f.fileno(), size)
        alloc = SmartSliceAllocator(mm, 0, 4)
        assert alloc.alloc_slice(1, 2) == 0
        alloc.mark(0)
        assert alloc.alloc_slice(1, 2) == 2
        with pytest.raises(MemoryError):
            alloc.alloc_slice(1)
        alloc.free_mark(0)
        assert alloc.alloc_slice(1, 2) == 0
        mm.close()
