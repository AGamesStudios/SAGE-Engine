import sage_engine.render as render


def test_specific_backend():
    backend = render.load_backend("headless")
    assert backend.__class__.__name__ == "HeadlessBackend"
