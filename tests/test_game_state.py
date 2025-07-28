from sage_engine.game_state import game_state


def test_game_state_stack():
    game_state.set_state("menu")
    assert game_state.current == "menu"
    game_state.push_state("play")
    assert game_state.current == "play"
    game_state.pop_state()
    assert game_state.current == "menu"
