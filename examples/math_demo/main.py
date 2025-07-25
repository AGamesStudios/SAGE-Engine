from sage_engine import core_boot, core_reset
from sage_engine.math import Vector2, vector_lerp, eval_expr, plot


def main() -> None:
    core_boot()
    v1 = Vector2(0, 0)
    v2 = Vector2(10, 10)
    print("lerp", vector_lerp(v1, v2, 0.25))
    print("expr", eval_expr("sin(pi/2) + 1"))
    print("points", plot("sin(x)", 0, 3.14, 1.57))
    core_reset()


if __name__ == "__main__":
    main()
