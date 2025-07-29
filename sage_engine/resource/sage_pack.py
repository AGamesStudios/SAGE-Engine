from __future__ import annotations

import argparse
import json
from pathlib import Path

from .packer import pack


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="SAGE pack compiler")
    sub = parser.add_subparsers(dest="cmd")
    build = sub.add_parser("build")
    build.add_argument("source")
    build.add_argument("output")
    build.add_argument("--optimize", action="store_true")
    build.add_argument("--report")
    args = parser.parse_args(argv)
    if args.cmd == "build":
        index = pack(args.source, args.output)
        if args.report:
            Path(args.report).write_text(json.dumps(index, indent=2))
        return 0
    parser.print_help()
    return 1


if __name__ == "__main__":  # pragma: no cover
    raise SystemExit(main())
