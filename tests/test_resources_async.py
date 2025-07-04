import asyncio
import zipfile

from engine.core.resources import ResourceManager


def test_import_file_async(tmp_path):
    src = tmp_path / "src.txt"
    src.write_text("data")
    rm = ResourceManager(str(tmp_path / "res"))
    rel = asyncio.run(rm.import_file_async(str(src)))
    assert (tmp_path / "res" / rel).read_text() == "data"


def test_import_folder_async(tmp_path):
    src_dir = tmp_path / "folder"
    src_dir.mkdir()
    (src_dir / "a.txt").write_text("a")
    rm = ResourceManager(str(tmp_path / "res"))
    rel = asyncio.run(rm.import_folder_async(str(src_dir)))
    dest = tmp_path / "res" / rel
    assert (dest / "a.txt").read_text() == "a"


def test_import_zip_async(tmp_path):
    src_dir = tmp_path / "zsrc"
    src_dir.mkdir()
    (src_dir / "f.txt").write_text("z")
    zip_path = tmp_path / "data.zip"
    with zipfile.ZipFile(zip_path, "w") as zf:
        zf.write(src_dir / "f.txt", arcname="f.txt")
    rm = ResourceManager(str(tmp_path / "res"))
    rel = asyncio.run(rm.import_zip_async(str(zip_path)))
    dest = tmp_path / "res" / rel
    assert (dest / "f.txt").read_text() == "z"


def test_load_data_async(tmp_path):
    (tmp_path / "file.bin").write_bytes(b"123")
    rm = ResourceManager(str(tmp_path))
    data = asyncio.run(rm.load_data_async("file.bin"))
    assert data == b"123"
