from __future__ import annotations

import argparse


def _build(_args: argparse.Namespace) -> None:
    pass


def _serve(_args: argparse.Namespace) -> None:
    pass


def _featherize(_args: argparse.Namespace) -> None:
    pass


_COMMANDS = {
    "build": _build,
    "serve": _serve,
    "featherize": _featherize,
}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="sage", description="SAGE command line")
    sub = parser.add_subparsers(dest="cmd", required=True)
    for name in _COMMANDS:
        sub.add_parser(name)
    args = parser.parse_args(argv)
    _COMMANDS[args.cmd](args)
    return 0


if __name__ == "__main__":  # pragma: no cover - entry point
    raise SystemExit(main())
