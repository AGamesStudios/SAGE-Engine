import argparse
import subprocess
import sys


def main() -> None:
    """Install SAGE Engine with optional extras and packages."""
    parser = argparse.ArgumentParser(
        description="Install the engine and optional components"
    )
    parser.add_argument(
        "--extras",
        nargs="*",
        default=[],
        help="Comma separated optional extras such as opengl,sdl,audio,qt,sdk",
    )
    parser.add_argument(
        "--package",
        action="append",
        default=[],
        help="Additional packages to install via pip",
    )
    parser.add_argument(
        "--editable",
        action="store_true",
        help="Install the engine in editable mode",
    )
    args = parser.parse_args()

    extras = ",".join(args.extras)
    command = [sys.executable, "-m", "pip", "install"]
    if args.editable:
        command += ["-e", "."]
    else:
        command.append(".")
    if extras:
        command[-1] += f"[{extras}]"

    subprocess.check_call(command)

    for pkg in args.package:
        subprocess.check_call([sys.executable, "-m", "pip", "install", pkg])


if __name__ == "__main__":
    main()
