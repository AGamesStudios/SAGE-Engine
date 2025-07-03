"""Helper imports for the ``sage_setup`` package."""

from typing import Iterable

from . import __main__ as _m

# Expose useful functions and constants so tests and callers can access them
DEFAULT_PATH = _m.DEFAULT_PATH
DEFAULT_PROJECTS = _m.DEFAULT_PROJECTS
REPO_ROOT = _m.REPO_ROOT
subprocess = _m.subprocess


def installed_versions(path: str = DEFAULT_PATH) -> list[str]:
    """Return engine versions installed under ``path``."""
    return _m.installed_versions(path)


def available_extras() -> list[str]:
    """Return optional extras defined for the package."""
    orig = _m.REPO_ROOT
    _m.REPO_ROOT = REPO_ROOT
    try:
        return _m.available_extras()
    finally:
        _m.REPO_ROOT = orig


def install_iter(
    target: str | None = DEFAULT_PATH,
    extras: str | None = None,
    version: str | None = None,
    *,
    launcher_only: bool = False,
) -> Iterable[tuple[str, int | None]]:
    return _m.install_iter(target, extras, version, launcher_only=launcher_only)


def install(
    target: str | None = DEFAULT_PATH,
    extras: str | None = None,
    version: str | None = None,
    *,
    launcher_only: bool = False,
) -> str:
    return _m.install(target, extras, version, launcher_only=launcher_only)


def run_install_dialog(*args, **kwargs):
    """Run the install dialog using the currently patched :func:`install_iter`."""
    original = _m.install_iter
    _m.install_iter = install_iter
    try:
        return _m.run_install_dialog(*args, **kwargs)
    finally:
        _m.install_iter = original


def main() -> None:
    """Entry point wrapper for ``python -m sage_setup``."""
    _m.main()


__all__ = [
    "main",
    "REPO_ROOT",
    "DEFAULT_PATH",
    "DEFAULT_PROJECTS",
    "installed_versions",
    "available_extras",
    "install_iter",
    "install",
    "run_install_dialog",
    "subprocess",
]
