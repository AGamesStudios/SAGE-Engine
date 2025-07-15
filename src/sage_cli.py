import argparse
import os
import runpy
from importlib import metadata


def list_backends() -> None:
    found = {ep.name for ep in metadata.entry_points(group="sage_gui")}
    known = ["qt6", "qt5", "tk"]
    names = sorted(found.union(known))
    for name in names:
        status = "installed" if name in found else "missing"
        print(f"{name} ({status})")


def cmd_run(args: argparse.Namespace) -> None:
    if args.gui == "list":
        list_backends()
        return
    if args.gui:
        os.environ["SAGE_GUI"] = args.gui
    runpy.run_path(args.script, run_name="__main__")


def main() -> None:
    parser = argparse.ArgumentParser(prog="sage")
    subparsers = parser.add_subparsers(dest="cmd")
    run_p = subparsers.add_parser("run", help="run a script")
    run_p.add_argument("script")
    run_p.add_argument("--gui", default="auto")
    args = parser.parse_args()
    if args.cmd == "run":
        cmd_run(args)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
