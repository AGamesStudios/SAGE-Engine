from pathlib import Path
from sage_engine.resource.sage_pack import main


def test_build_pack(tmp_path: Path):
    src = tmp_path / "src"
    src.mkdir()
    (src / "a.bin").write_bytes(b'd')
    out = tmp_path / "r.sagepack"
    assert main(["build", str(src), str(out)]) == 0
    assert out.exists()
