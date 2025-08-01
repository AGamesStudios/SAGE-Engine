def test_render_software():
    from sage_engine.render import _get_backend
    from sage_engine.render.backends import software
    backend = software.get_backend()
    assert backend is not None

