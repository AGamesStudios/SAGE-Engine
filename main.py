"""Entry point for SAGE Engine."""

from configparser import ConfigParser

from sage_engine.logger import setup_logging
from sage_engine.core import boot_engine


def load_config() -> dict:
    parser = ConfigParser()
    parser.read("engine.sagecfg")
    mode = parser.get("run", "mode", fallback="window")
    return {"mode": mode}


if __name__ == "__main__":
    setup_logging(level="info")
    cfg = load_config()
    if cfg.get("mode") == "tty":
        from sage_engine import tty  # noqa: F401
    else:
        from sage_engine import window, render, graphic  # noqa: F401
    boot_engine(cfg)
