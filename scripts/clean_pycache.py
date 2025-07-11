#!/usr/bin/env python3
"""Remove __pycache__ directories from the given path."""
import os
import sys

def clean(root: str) -> None:
    for dirpath, dirnames, _ in os.walk(root):
        for d in list(dirnames):
            if d == "__pycache__":
                path = os.path.join(dirpath, d)
                try:
                    for file in os.listdir(path):
                        os.remove(os.path.join(path, file))
                except FileNotFoundError:
                    pass
                try:
                    os.rmdir(path)
                except OSError:
                    pass

if __name__ == "__main__":
    clean(sys.argv[1] if len(sys.argv) > 1 else ".")
