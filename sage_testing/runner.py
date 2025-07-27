"""SAGE Testing runner."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import pytest

from .collect import collect_tests
from .performance import run_performance_checks


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run SAGE tests")
    parser.add_argument("paths", nargs="*", default=["tests"], help="Test paths")
    parser.add_argument("--perf", action="store_true", help="Run performance checks")
    parser.add_argument("--report", type=Path, help="Output JSON report")
    args = parser.parse_args(argv)

    collected = collect_tests(args.paths)
    result = pytest.main(collected)

    report_data = {"result": result}
    if args.perf:
        report_data["performance"] = run_performance_checks()
    if args.report:
        args.report.write_text(json.dumps(report_data, indent=2))
    return result


if __name__ == "__main__":
    sys.exit(main())
