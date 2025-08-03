import argparse
import json
from typing import Optional
import yaml
from pathlib import Path
import subprocess
import sys
import shutil
import importlib.resources as pkg_resources

from ..format import (
    SAGECompiler,
    SAGEDecompiler,
    SAGESchemaSystem,
    pack_directory,
)




def format_compile(src: Path, dst: Path) -> None:
    SAGECompiler().compile(src, dst)


def format_decompile(src: Path, dst: Optional[Path]) -> None:
    data = SAGEDecompiler().decompile(src)
    if dst:
        dst.write_text(yaml.safe_dump(data), encoding="utf8")
    else:
        print(yaml.safe_dump(data))


def convert_file(src: Path, dst: Path) -> None:
    """Convert legacy YAML/JSON/etc. to a SAGE binary file."""
    SAGECompiler().compile(src, dst)


def validate_file(path: Path, schema: Path) -> None:
    system = SAGESchemaSystem()
    spec = yaml.safe_load(schema.read_text(encoding="utf8"))
    system.register("tmp", spec)
    data = SAGEDecompiler().decompile(path)
    system.validate("tmp", data)
    print("OK")


def pack_dir(src: Path, dst: Path) -> None:
    pack_directory(src, dst)


def debug_stats() -> None:
    """Print live render statistics."""
    from ..render import stats as render_stats
    print(json.dumps(render_stats.stats, indent=2))


def transform_info() -> None:
    """Print transform statistics."""
    from ..transform import stats as tstats

    print(json.dumps(tstats.stats, indent=2))


def build_assets_call(path: Path) -> None:
    """Run the build_assets.py helper script."""
    script = Path(__file__).resolve().parents[2] / "tools" / "build_assets.py"
    subprocess.run([sys.executable, str(script), str(path)], check=True)


def new_project(name: str, template: str) -> Path:
    """Create a new project from *template*."""
    dst = Path(name)
    tpl_root = pkg_resources.files("sage_engine.resources.templates") / template
    shutil.copytree(tpl_root, dst)
    return dst


def init_project(path: Path, template: str) -> None:
    tpl_root = pkg_resources.files("sage_engine.resources.templates") / template
    for item in tpl_root.iterdir():
        dst = path / item.name
        if item.is_dir():
            shutil.copytree(item, dst, dirs_exist_ok=True)
        else:
            shutil.copy2(item, dst)


def run_project(path: Path) -> None:
    main_py = path / "main.py"
    subprocess.run([sys.executable, str(main_py)], check=True)


def check_env() -> None:
    print(f"Python {sys.version_info.major}.{sys.version_info.minor} ✓")
    default_font = pkg_resources.files("sage_engine.resources.fonts") / "default.ttf"
    print(f"Default font fallback {'✓' if default_font.exists() else 'missing'}")


def run_tests() -> None:
    subprocess.run([sys.executable, "-m", "pytest", "-q"], check=True)


def main(argv: Optional[list[str]] = None) -> None:
    parser = argparse.ArgumentParser(prog="sage")
    sub = parser.add_subparsers(dest="topic")

    new = sub.add_parser("new")
    new.add_argument("name")
    new.add_argument("--template", default="blank-2d")

    initp = sub.add_parser("init")
    initp.add_argument("--name")
    initp.add_argument("--template", default="blank-2d")

    run = sub.add_parser("run")
    run.add_argument("path", nargs="?", default=".")

    chkenv = sub.add_parser("check")

    testr = sub.add_parser("test")

    tpl = sub.add_parser("template")
    tpl.add_argument("cmd", choices=["list"])

    cfg = sub.add_parser("config")
    cfg_sub = cfg.add_subparsers(dest="cmd")
    cfg_get = cfg_sub.add_parser("get")
    cfg_get.add_argument("key")
    cfg_get.add_argument("--file", default="engine.sagecfg")
    cfg_set = cfg_sub.add_parser("set")
    cfg_set.add_argument("key")
    cfg_set.add_argument("value")
    cfg_set.add_argument("--file", default="engine.sagecfg")

    grp = sub.add_parser("groups")
    grp_sub = grp.add_subparsers(dest="cmd")
    grp_list = grp_sub.add_parser("list")
    grp_list.add_argument("--config", default="engine.sagecfg")
    grp_apply = grp_sub.add_parser("apply")
    grp_apply.add_argument("--config", required=True)
    grp_apply.add_argument("--add", action="append", default=[])

    # legacy commands removed: blueprint migrate, compat check

    fmt = sub.add_parser("format")
    fmt_sub = fmt.add_subparsers(dest="cmd")
    fc = fmt_sub.add_parser("compile")
    fc.add_argument("src")
    fc.add_argument("dst")
    fd = fmt_sub.add_parser("decompile")
    fd.add_argument("src")
    fd.add_argument("dst", nargs="?")

    conv = sub.add_parser("convert")
    conv.add_argument("src")
    conv.add_argument("dst")

    val = sub.add_parser("validate")
    val.add_argument("path")
    val.add_argument("--schema", required=True)

    pack = sub.add_parser("pack")
    pack.add_argument("src")
    pack.add_argument("dst")

    debug = sub.add_parser("debug")
    debug_sub = debug.add_subparsers(dest="cmd")
    debug_sub.add_parser("stats")

    info = sub.add_parser("info")
    info_sub = info.add_subparsers(dest="cmd")
    info_sub.add_parser("transform")

    build = sub.add_parser("build-assets")
    build.add_argument("path", nargs="?", default=".")

    args = parser.parse_args(argv)

    if args.topic == "new":
        new_project(args.name, args.template)
    elif args.topic == "init":
        init_project(Path.cwd(), args.template)
    elif args.topic == "run":
        run_project(Path(args.path))
    elif args.topic == "check":
        check_env()
    elif args.topic == "test":
        run_tests()
    elif args.topic == "template" and args.cmd == "list":
        tpl_root = pkg_resources.files("sage_engine.resources.templates")
        for d in tpl_root.iterdir():
            if d.is_dir():
                print(d.name)
    elif args.topic == "config" and args.cmd == "get":
        cfg = Path(args.file).read_text(encoding="utf8")
        for line in cfg.splitlines():
            if line.startswith(args.key):
                print(line.split("=", 1)[1].strip())
    elif args.topic == "config" and args.cmd == "set":
        cfg_path = Path(args.file)
        lines = cfg_path.read_text(encoding="utf8").splitlines()
        new_lines = []
        found = False
        for ln in lines:
            if ln.startswith(args.key):
                new_lines.append(f"{args.key} = {args.value}")
                found = True
            else:
                new_lines.append(ln)
        if not found:
            new_lines.append(f"{args.key} = {args.value}")
        cfg_path.write_text("\n".join(new_lines), encoding="utf8")
    elif args.topic == "groups" and args.cmd == "list":
        text = Path(args.config).read_text(encoding="utf8")
        print(text)
    elif args.topic == "groups" and args.cmd == "apply":
        cfg_path = Path(args.config)
        text = cfg_path.read_text(encoding="utf8").splitlines()
        for a in args.add:
            text.append(a)
        cfg_path.write_text("\n".join(text), encoding="utf8")
    elif args.topic == "format" and args.cmd == "compile":
        format_compile(Path(args.src), Path(args.dst))
    elif args.topic == "format" and args.cmd == "decompile":
        format_decompile(Path(args.src), Path(args.dst) if args.dst else None)
    elif args.topic == "convert":
        convert_file(Path(args.src), Path(args.dst))
    elif args.topic == "validate":
        validate_file(Path(args.path), Path(args.schema))
    elif args.topic == "pack":
        pack_dir(Path(args.src), Path(args.dst))
    elif args.topic == "debug" and args.cmd == "stats":
        debug_stats()
    elif args.topic == "info" and args.cmd == "transform":
        transform_info()
    elif args.topic == "build-assets":
        build_assets_call(Path(args.path))
    else:
        parser.print_help()


if __name__ == "__main__":  # pragma: no cover
    main()
