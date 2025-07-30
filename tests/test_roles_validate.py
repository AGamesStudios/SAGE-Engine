import pytest
from sage_engine import roles


def test_validate_ok():
    roles.validate("sprite", {"x": 1.0, "y": 2.0, "angle": 0.0, "tex_id": 1, "layer": 0, "color": [1,1,1,1]})


def test_validate_error():
    with pytest.raises(TypeError):
        roles.validate("sprite", {"x": "bad"})
