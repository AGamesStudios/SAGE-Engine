from sage_engine.math import Vector2, Vector3, Matrix3, eval_expr, vector_lerp, plot


def test_vector_ops():
    v1 = Vector2(1, 2)
    v2 = Vector2(3, 4)
    assert (v1 + v2).to_tuple() == (4, 6)
    assert (v2 - v1).to_tuple() == (2, 2)
    assert (v1 * 2).to_tuple() == (2, 4)


def test_matrix_mul():
    m = Matrix3([[1, 0, 0], [0, 1, 0], [0, 0, 1]])
    v = Vector3(1, 2, 3)
    assert (m @ v).to_tuple() == (1, 2, 3)


def test_eval_expr_and_plot():
    assert eval_expr("sin(pi/2)") == 1.0
    pts = plot("x", 0, 1, 0.5)
    assert pts == [(0, 0.0), (0.5, 0.5), (1.0, 1.0)]


def test_vector_lerp():
    v1 = Vector2(0, 0)
    v2 = Vector2(10, 0)
    mid = vector_lerp(v1, v2, 0.5)
    assert mid.to_tuple() == (5, 0)
