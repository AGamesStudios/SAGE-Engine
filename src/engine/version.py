"""Version information for the SAGE engine."""

from packaging.version import Version

DEVELOPMENT_STATUS = "В разработке"

__version__ = "0.0.1-alpha"

FULL_VERSION = f"{__version__} ({DEVELOPMENT_STATUS})"


def require(min_version: str) -> None:
    """Raise ``RuntimeError`` if the engine is older than ``min_version``."""
    if Version(__version__) < Version(min_version):
        raise RuntimeError(
            f"SAGE Engine {min_version} or newer required (found {__version__})"
        )
