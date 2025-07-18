import argparse
import compileall
import lzma
import os
import shutil
from pathlib import Path


def build_core(output: str = "dist/sage_core.lzma") -> Path:
    root = Path(__file__).resolve().parents[1]
    build_dir = root / "build" / "core_pycache"
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True)

    compileall.compile_dir(root / "sage_engine", force=True, quiet=1, optimize=2, ddir="sage_engine", destdir=build_dir)
    compileall.compile_dir(root / "sage_object", force=True, quiet=1, optimize=2, ddir="sage_object", destdir=build_dir)

    archive = build_dir.with_suffix('.zip')
    shutil.make_archive(str(build_dir), 'zip', build_dir)

    out_path = root / output
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(archive, 'rb') as f_in, lzma.open(out_path, 'wb') as f_out:
        f_out.write(f_in.read())

    shutil.rmtree(build_dir)
    os.remove(archive)
    print(f"Built {out_path}")
    return out_path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', default='dist/sage_core.lzma')
    args = parser.parse_args()
    build_core(args.output)


if __name__ == '__main__':
    main()
