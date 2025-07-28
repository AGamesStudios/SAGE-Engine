"""Render backend implementations."""

from .software import SoftwareBackend
from .opengl import OpenGLBackend
from .vulkan import VulkanBackend

__all__ = ["get_backend"]


def get_backend(name: str = "software"):
    if name == "opengl":
        return OpenGLBackend()
    if name == "vulkan":
        return VulkanBackend()
    return SoftwareBackend()
