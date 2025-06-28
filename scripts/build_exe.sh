#!/bin/sh
# Bundle the editor with all its icons so the executable runs standalone.
python -m PyInstaller \
    --name SAGE-Engine \
    --onefile \
    --windowed \
    --icon sage_editor/icons/icon.ico \
    --add-data "sage_editor/icons:sage_editor/icons" \
    main.py
