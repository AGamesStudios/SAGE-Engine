from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import pytest

from .performance import run_performance_checks


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run SAGE tests")
    parser.add_argument("paths", nargs="*", default=["tests"], help="Test paths")
    parser.add_argument("--report", type=Path, help="Write JSON report")
    parser.add_argument("--perf", action="store_true", help="Include perf stats")
    args = parser.parse_args(argv)

    result = pytest.main(args.paths)
    if args.report:
        data = {"result": result}
        if args.perf:
            data["performance"] = run_performance_checks()
        args.report.write_text(json.dumps(data, indent=2))
    return result


if __name__ == "__main__":
    sys.exit(main())
