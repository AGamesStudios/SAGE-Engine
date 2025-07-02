# File Formats

SAGE Engine stores scenes in `.sagescene` files and projects in `.sageproject` files.
Custom resources use the following compact formats:

* `.sageaudio` – JSON descriptor containing a `file` entry pointing to a sound file and optional metadata.
* `.sagemesh` – JSON mesh data listing `vertices` and optional `indices`.

These text formats keep resources small and easy to edit.
