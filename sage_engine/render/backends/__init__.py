"""Render backend implementations."""

from .software import SoftwareBackend
from .vulkan import VulkanBackend

__all__ = ["get_backend"]


def get_backend(name: str = "software"):
    if name == "vulkan":
        return VulkanBackend()
    return SoftwareBackend()
