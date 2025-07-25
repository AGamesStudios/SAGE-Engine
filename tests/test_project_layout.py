from pathlib import Path

import importlib.util
from pathlib import Path as _Path

spec = importlib.util.spec_from_file_location(
    "sage_engine.project", _Path("sage_engine/project.py")
)
project = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(project)
ensure_layout = project.ensure_layout
validate_structure = project.validate_structure


def test_ensure_and_validate(tmp_path: Path) -> None:
    ensure_layout(str(tmp_path))
    missing = validate_structure(str(tmp_path))
    assert missing == []


def test_validate_missing(tmp_path: Path) -> None:
    (tmp_path / "main.py").write_text("", encoding="utf-8")
    missing = validate_structure(str(tmp_path))
    assert "config.yaml" in missing
