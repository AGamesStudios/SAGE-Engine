#!/usr/bin/env python3
"""Simple documentation builder using the Markdown package."""
from pathlib import Path
import shutil
import markdown

OUT_DIR = Path("site")


def build():
    if OUT_DIR.exists():
        shutil.rmtree(OUT_DIR)
    OUT_DIR.mkdir()
    for md_file in Path("docs").rglob("*.md"):
        html = markdown.markdown(md_file.read_text())
        target = OUT_DIR / md_file.relative_to("docs").with_suffix(".html")
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(html)
    print(f"Docs built to {OUT_DIR}")


if __name__ == "__main__":
    build()
