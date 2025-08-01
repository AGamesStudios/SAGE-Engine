import os
from sage_engine import render


def test_select_vulkan_backend(monkeypatch):
    monkeypatch.setenv("SAGE_RENDER_BACKEND", "vulkan")
    render.init(None)
    assert render._get_backend().__class__.__name__ == "VulkanBackend"
    render.shutdown()
