from sage_engine.graphic import state


def test_state_set_get():
    state.set_state("style", "neo-retro")
    assert state.get_state("style") == "neo-retro"


def test_state_clear():
    state.set_state("foo", "bar")
    state.clear_state()
    assert state.get_state("foo") is None
