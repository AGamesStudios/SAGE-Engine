import pytest
from sage_engine import resource
from sage_engine.resource import loader, packer, cache, manager


def test_pack_deduplicates(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    (src / "a.txt").write_text("data", encoding="utf8")
    (src / "b.txt").write_text("data", encoding="utf8")
    pack = tmp_path / "r.sagepack"
    idx = packer.pack(str(src), str(pack), limit_size_mb=1)
    assert idx["a.txt"]["offset"] == 0
    assert idx["b.txt"]["offset"] == 0
    loader._PACK_INDEX = None
    loader._PACK_FILE = None
    manager.configure(str(pack))
    assert resource.load("a.txt") == b"data"
    assert resource.load("b.txt") == b"data"


def test_pack_limit_size(tmp_path):
    src = tmp_path / "src2"
    src.mkdir()
    big = src / "big.bin"
    big.write_bytes(b"0" * (2 * 1024 * 1024))
    with pytest.raises(ValueError):
        packer.pack(str(src), str(tmp_path / "p.sagepack"), limit_size_mb=1)


def test_cache_clear(tmp_path):
    f = tmp_path / "file.bin"
    f.write_text("hello")
    manager.configure(None)
    data1 = resource.load(str(f))
    cache.clear()
    data2 = resource.load(str(f))
    assert data1 == data2 and data1 is not data2


def test_load_from_pack(tmp_path):
    src = tmp_path / "src3"
    src.mkdir()
    (src / "c.txt").write_text("zzz")
    pack = tmp_path / "s.sagepack"
    packer.pack(str(src), str(pack), limit_size_mb=1)
    loader._PACK_INDEX = None
    loader._PACK_FILE = None
    data = loader.load_from_pack("c.txt", pack)
    assert data == b"zzz"
