#!/usr/bin/env python3
"""Build documentation using MkDocs."""

import subprocess


def build() -> None:
    subprocess.run(["mkdocs", "build", "--clean"], check=True)


if __name__ == "__main__":
    build()
