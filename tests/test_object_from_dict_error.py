import pytest
from engine.core.objects import object_from_dict


def test_unknown_object_error():
    with pytest.raises(ValueError):
        object_from_dict({"type": "does_not_exist"})
