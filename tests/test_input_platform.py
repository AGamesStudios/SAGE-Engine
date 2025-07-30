import importlib

from sage_engine.input.impl import x11, cocoa


def test_x11_impl_import():
    assert hasattr(x11, "get_key_state")
    assert x11.get_key_state("LEFT") is False


def test_cocoa_impl_import():
    assert hasattr(cocoa, "get_key_state")
    assert cocoa.get_key_state("LEFT") is False
