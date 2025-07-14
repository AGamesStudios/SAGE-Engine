# Contributing

Thank you for helping improve SAGE Engine. Keep commit messages short and descriptive so the history clearly reflects what changed. Summarise new behaviour, e.g. "Add PyQt6-based installer and launcher with extras selection". See `AGENTS.md` for other project rules. Always run `ruff check .` and `PYTHONPATH=. pytest -q` before committing.

One feature should be implemented per pull request. Each PR must add a line to `CHANGELOG.md` summarising the change. CI fails if the changelog is not updated.
