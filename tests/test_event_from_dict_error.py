import pytest
from engine.logic.base import event_from_dict


def test_invalid_condition_error():
    with pytest.raises(ValueError):
        event_from_dict({"conditions": [{"type": "does_not_exist"}]}, [], {})

def test_invalid_action_error():
    with pytest.raises(ValueError):
        event_from_dict({"actions": [{"type": "nope"}]}, [], {})
