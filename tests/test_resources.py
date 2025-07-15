import zipfile
from sage_engine.core.resources import ResourceManager


def test_import_file(tmp_path):
    src = tmp_path / "src.txt"
    src.write_text("data")
    rm = ResourceManager(str(tmp_path / "res"))
    rel = rm.import_file(str(src))
    assert (tmp_path / "res" / rel).read_text() == "data"


def test_import_folder(tmp_path):
    src_dir = tmp_path / "folder"
    src_dir.mkdir()
    (src_dir / "a.txt").write_text("a")
    rm = ResourceManager(str(tmp_path / "res"))
    rel = rm.import_folder(str(src_dir))
    dest = tmp_path / "res" / rel
    assert (dest / "a.txt").read_text() == "a"


def test_import_zip(tmp_path):
    src_dir = tmp_path / "zsrc"
    src_dir.mkdir()
    (src_dir / "f.txt").write_text("z")
    zip_path = tmp_path / "data.zip"
    with zipfile.ZipFile(zip_path, "w") as zf:
        zf.write(src_dir / "f.txt", arcname="f.txt")
    rm = ResourceManager(str(tmp_path / "res"))
    rel = rm.import_zip(str(zip_path))
    dest = tmp_path / "res" / rel
    assert (dest / "f.txt").read_text() == "z"


def test_import_zip_no_traversal(tmp_path):
    z = tmp_path / "evil.zip"
    with zipfile.ZipFile(z, "w") as zf:
        zf.writestr("../evil.txt", "bad")
    rm = ResourceManager(str(tmp_path / "res"))
    rm.import_zip(str(z))
    assert not (tmp_path / "evil.txt").exists()
