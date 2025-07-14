#!/usr/bin/env python3
import argparse
import pathlib
import re
import sys

parser = argparse.ArgumentParser()
parser.add_argument("message_or_path")
parser.add_argument(
    "--message", action="store_true", help="treat argument as message text"
)
args = parser.parse_args()

if args.message:
    msg = args.message_or_path
else:
    msg = pathlib.Path(args.message_or_path).read_text()

if not re.match(r"^(feat|fix|docs|refactor):", msg.strip()):
    print(
        "Commit message must start with 'feat', 'fix', 'docs', or 'refactor:'",
        file=sys.stderr,
    )
    raise SystemExit(1)
