"""Role registry for SAGE Object system."""

from __future__ import annotations

from typing import Dict, Type

from .interfaces import Role

_ROLE_REGISTRY: Dict[str, Type[Role]] = {}


def register(name: str, cls: Type[Role]) -> None:
    _ROLE_REGISTRY[name] = cls


def get(name: str) -> Type[Role]:
    return _ROLE_REGISTRY[name]


def registered() -> Dict[str, Type[Role]]:
    return dict(_ROLE_REGISTRY)


from . import builtins  # noqa: E402,F401
