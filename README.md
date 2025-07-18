# SAGE Feather

This repository contains **SAGE Feather**, a minimal runtime for the SAGE Engine written in Rust.
It exposes a simple C API that can be used from Python or other languages.

The library includes a ChronoPatch Tree for state storage, a DAG scheduler for tasks
and bindings for running MicroPython scripts.

## Building

```bash
cargo build --release --manifest-path rust/feather_core/Cargo.toml
```

The library targets Python 3.8 through 3.13 via PyO3. Set
`PYO3_USE_ABI3_FORWARD_COMPATIBILITY=1` when using newer interpreters.

## Example

Run the Alpha demo to see Feather in action:

```bash
python examples/alpha_demo/main.py
```

This example creates a ChronoPatch Tree, patches object velocity,
executes a MicroPython script via a DAG task and prints the final state.

## Tests

Run the code style check and unit tests with:

```bash
ruff check .
PYTHONPATH=. pytest -q
```

The checklist for the Alpha release lives in
[SAGE_ALPHA_1.0_checklist.md](SAGE_ALPHA_1.0_checklist.md).
