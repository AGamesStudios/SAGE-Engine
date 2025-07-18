# SAGE Feather Alpha 1.0 Checklist

Use this list to verify that the minimal runtime is stable before declaring the
Alpha 1.0 release.

- [x] **Tests**: `ruff check .` and `pytest -q` pass on a clean environment.
- [x] **PyO3**: Builds successfully against Python 3.8–3.13.
- [x] **ChronoPatchTree**: Patch apply/revert works without data loss.
- [x] **DAG Scheduler**: Executes tasks in the expected order and detects cycles.
- [x] **MicroPython**: Scripts execute and modify state via the FFI helpers.
- [x] **Documentation**: README and guides reflect current behaviour.

Once these tasks are complete, remove the "Candidate" label from the README and
mark the repository as **Alpha 0.2**.
