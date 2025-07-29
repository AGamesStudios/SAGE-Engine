from sage_engine.graphic import api, state


def test_config_load_dict():
    cfg = {
        "antialiasing": "EdgeAverage2x",
        "style": "mono-dark",
    }
    state.config.load_from_dict(cfg)
    assert state.config.antialiasing == "EdgeAverage2x"
    assert state.config.style == "mono-dark"


def test_api_draw_and_flush():
    api.init(2, 2)
    api.draw_line(0, 0, 1, 1, (255, 0, 0, 255))
    buf = api.flush()
    assert len(buf) == 2 * 2 * 4

