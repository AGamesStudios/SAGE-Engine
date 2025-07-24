import os
import sys
import yaml

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../..")))

from sage.config import DEFAULT_WINDOW_CONFIG
from sage_engine import core_boot, core_reset
from sage_engine.math import Vector2, vector_lerp, eval_expr, plot


def load_cfg() -> None:
    path = os.path.join(os.path.dirname(__file__), "config.yaml")
    if os.path.isfile(path):
        data = yaml.safe_load(open(path, encoding="utf-8")) or {}
        DEFAULT_WINDOW_CONFIG.update(data.get("window", {}))


def main() -> None:
    load_cfg()
    core_boot()
    v1 = Vector2(0, 0)
    v2 = Vector2(10, 10)
    print("lerp", vector_lerp(v1, v2, 0.25))
    print("expr", eval_expr("sin(pi/2) + 1"))
    print("points", plot("sin(x)", 0, 3.14, 1.57))
    core_reset()


if __name__ == "__main__":
    main()
