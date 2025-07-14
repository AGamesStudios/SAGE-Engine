# Migration Wizard

`sage migrate` scans a project directory for `.sageproject` and `.sagescene` files.
It renames legacy fields and adds missing `version` keys. The command prints the
path of each file it updates.
