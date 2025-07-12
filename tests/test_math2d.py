import pytest
from engine.core.math2d import (
    make_transform,
    multiply_matrix,
    decompose_matrix,
)
from engine.entities.object import Transform2D


def test_matrix_multiply_and_decompose():
    mat = make_transform(1, 2, angle=45)
    ident = make_transform()
    combined = multiply_matrix(mat, ident)
    x, y, sx, sy, ang = decompose_matrix(combined)
    assert x == pytest.approx(1)
    assert y == pytest.approx(2)
    assert sx == pytest.approx(1)
    assert sy == pytest.approx(1)
    assert ang == pytest.approx(45)


def test_transform_apply_matrix():
    t = Transform2D(x=0, y=0)
    mat = make_transform(2, 3)
    t.apply_matrix(mat)
    assert (t.x, t.y) == (2, 3)

