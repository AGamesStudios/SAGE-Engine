# Guidelines for SAGE Engine contributors

This project contains a modular game engine (`engine`), an optional editor (`sage_editor`) and supporting tools. The editor relies on the engine but the engine must remain independent from the editor so it can run on its own or via the `engine.runtime` package.

## Rules
- **No binary assets**. Do not commit `.png`, `.jpg`, `.gif`, `.ico` or other non-text files. Reference icons by filename only.
- **Keep packages modular.** Engine code should never import from `sage_editor`. Extensions should be optional and loaded via entry points or plugins.
- **Testing**: Run `ruff check .` and `PYTHONPATH=. pytest -q` before every commit. Update tests when changing behaviour.
- **Documentation**: Keep `README.md` concise. Place detailed guides under `docs/` and link to them from the README.
- **Commits**: Write descriptive commit messages that summarise the change.
- **Custom style only.** Never use Qt's built-in styles such as "Fusion".
  The editor must apply its own palette and stylesheet.
- **Release checklist**: Consult `SAGE_ALPHA_1.0_checklist.md` for tasks that
  must be completed before the Alpha release.

Following these rules helps keep the engine lightweight, extensible and easy to maintain.
