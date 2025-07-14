# Migration Wizard

`sage migrate` scans a project directory for `.sageproject` and `.sagescene` files written in YAML (JSON also works).
It renames legacy fields such as `scene` to `scene_file`, inserts a `version` key and writes the file back. Each changed file is listed in the output.
