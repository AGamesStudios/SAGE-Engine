import asyncio
from sage_engine import resource


def test_load_async(tmp_path):
    p = tmp_path / "file.txt"
    p.write_text("hello", encoding="utf8")
    data = asyncio.run(resource.load_async(str(p)))
    assert data == b"hello"


def test_pack_and_load(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    f = src / "a.bin"
    f.write_bytes(b"data")
    pack = tmp_path / "r.sagepack"
    resource.pack(str(src), str(pack), limit_size_mb=1)
    resource.manager.configure(str(pack))
    out = resource.load("a.bin")
    assert out == b"data"


def test_cache(tmp_path):
    f = tmp_path / "b.txt"
    f.write_text("c")
    resource.manager.configure(None)
    first = resource.load(str(f))
    second = resource.load(str(f))
    assert first is second
