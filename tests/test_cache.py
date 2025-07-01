import tempfile
from engine.core.resources import ResourceManager
from engine.cache import SAGE_CACHE


def test_shared_cache(tmp_path):
    file_path = tmp_path / "data.txt"
    file_path.write_text("hello")
    rm1 = ResourceManager(str(tmp_path))
    data1 = rm1.load_data("data.txt")
    assert data1 == b"hello"
    # second manager should hit global cache
    rm2 = ResourceManager(str(tmp_path))
    rm2._cache.clear()
    data2 = rm2.load_data("data.txt")
    assert data2 == b"hello"
    assert SAGE_CACHE.get(str(file_path)) == b"hello"
