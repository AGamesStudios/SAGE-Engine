# SAGE Engine

**SAGE Engine** provides a lightweight runtime and optional Qt based editor. Scenes are stored in the `.sagescene` format and full projects in `.sageproject` files.

Documentation lives under `docs/`. Start with [docs/en/index.md](docs/en/index.md) for an overview, see [plugins](docs/en/plugins.md) for extension points and [optimisation tips](docs/en/optimisation.md) for performance advice.

## Quick start
```bash
pip install -r requirements.txt
ruff check .
PYTHONPATH=. pytest -q
```

Open `examples/blank.sageproject` with the editor or runtime to see the basic structure.

## Running
Use `python -m engine path/to/project.sageproject` to launch a game or `python main.py` for the editor. Both use the OpenGL renderer by default but fall back to the NullRenderer in headless setups.

See the documentation for more details on object types, input backends and renderer options.

## License
Released under the [MIT License](LICENSE). Contributions are welcome! Please keep commit messages descriptive and run the tests before submitting patches. See [CONTRIBUTING.md](CONTRIBUTING.md).
