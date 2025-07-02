#!/bin/sh
# Build the editor as a standalone executable.
python -m PyInstaller --name SAGE-Engine --onefile --windowed main.py
