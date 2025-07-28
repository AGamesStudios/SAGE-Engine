import asyncio
from sage_engine import resource


def test_load_async(tmp_path):
    p = tmp_path / "file.txt"
    p.write_text("hello", encoding="utf8")
    data = asyncio.run(resource.load_async(str(p)))
    assert data == b"hello"
