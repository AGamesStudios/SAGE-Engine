from __future__ import annotations

"""Compile raw asset files into `.sage*` binaries."""

import argparse
from pathlib import Path

from sage_engine.format import SAGECompiler


COMPILER = SAGECompiler()


def compile_file(src: Path) -> None:
    """Compile YAML asset to binary with the same stem."""
    if src.suffix not in {".yml", ".yaml"}:
        return
    dst = src.with_suffix("")  # drop .yml extension
    COMPILER.compile(src, dst)
    print(f"Compiled {src} -> {dst}")


def build_assets(base: Path) -> None:
    for raw in base.rglob("raw"):
        for src in raw.glob("*.yml"):
            compile_file(src)


def main(argv: list[str] | None = None) -> None:
    parser = argparse.ArgumentParser(description="Compile raw assets")
    parser.add_argument("path", nargs="?", default=".", help="Base path")
    args = parser.parse_args(argv)
    build_assets(Path(args.path))


if __name__ == "__main__":  # pragma: no cover
    main()
