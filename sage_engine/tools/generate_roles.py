"""Simple role code generator (stub)."""

import importlib
import sys
from pathlib import Path

TEMPLATE = """# Auto-generated file\nROLE_{name_upper} = '{name}'\n"""


def generate(role_module: str, out_dir: str) -> None:
    mod = importlib.import_module(role_module)
    name = Path(role_module).stem.replace('_schema', '')
    out_path = Path(out_dir) / f"{name}.py"
    with open(out_path, "w", encoding="utf8") as fh:
        fh.write(TEMPLATE.format(name=name, name_upper=name.upper()))
    print(f"Generated {out_path}")


if __name__ == "__main__":
    generate(sys.argv[1], sys.argv[2])
