from pathlib import Path

import yaml

from sage_engine.format import SAGECompiler, SAGEDecompiler
from sage_engine.objects import ObjectBuilder, ObjectStore, BlueprintSystem


def test_roundtrip(tmp_path: Path):
    data = {"sprite_id": 3, "visible": True}
    src = tmp_path / "obj.yaml"
    src.write_text(yaml.dump(data), encoding="utf8")
    out = tmp_path / "obj.sageobj"
    SAGECompiler().compile(src, out)
    assert out.exists()
    loaded = SAGEDecompiler().decompile(out)
    assert loaded == data


def test_builder_from_sagebp(tmp_path: Path):
    blueprint = {
        "name": "simple",
        "roles": [],
    }
    bp_yaml = tmp_path / "bp.yaml"
    bp_yaml.write_text(yaml.dump(blueprint), encoding="utf8")
    bp_bin = tmp_path / "bp.sagebp"
    SAGECompiler().compile(bp_yaml, bp_bin)

    system = BlueprintSystem()
    system.load(bp_bin)
    store = ObjectStore()
    builder = ObjectBuilder(store, system)
    obj = builder.build("simple")
    assert obj.name == "simple"
