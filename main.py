"""Entry point for SAGE Engine."""

from configparser import ConfigParser

from sage_engine.logger import setup_logging
from sage_engine.core import boot_engine


def load_config() -> dict:
    parser = ConfigParser()
    parser.read("engine.sagecfg")
    mode = parser.get("run", "mode", fallback="window")
    ascii_ui = parser.getboolean("run", "ascii_ui", fallback=False)
    theme = parser.get("run", "theme", fallback="tty_dark")
    return {"mode": mode, "ascii_ui": ascii_ui, "theme": theme}


if __name__ == "__main__":
    setup_logging(level="info")
    cfg = load_config()
    if cfg.get("mode") == "tty":
        from sage_engine import tty  # noqa: F401
        if cfg.get("ascii_ui"):
            from sage_engine.tty import ui_core
            ui_core.init({"theme": cfg.get("theme")})
    else:
        from sage_engine import window, render, graphic  # noqa: F401
    boot_engine(cfg)
