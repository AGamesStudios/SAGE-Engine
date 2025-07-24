# SAGE Math

The ``sage_engine.math`` module provides small helpers used by the
engine and scripting languages. Vectors and matrices are implemented in
pure Python and can be used from FlowScript, Lua and Python scripts.

## Vectors

Use ``Vector2`` or ``Vector3`` to represent positions and directions.
They support addition, subtraction and scalar multiplication.

```python
from sage_engine.math import Vector2, vector_lerp

v1 = Vector2(0, 0)
v2 = Vector2(10, 10)
print(vector_lerp(v1, v2, 0.5))  # Vector2(x=5, y=5)
```

## Expressions

``eval_expr(expr, **vars)`` evaluates a mathematical expression in a
restricted environment with access to functions from ``math``.

```python
from sage_engine.math import eval_expr
value = eval_expr("sin(pi / 4) * x", x=2)
```

## Plotting

``plot(expr, start, end, step)`` returns a list of ``(x, y)`` points for
the given expression. You can use these points to draw graphs or debug
particle behaviour.

```python
from sage_engine.math import plot
points = plot("sin(x)", 0, 3.14, 0.5)
```
