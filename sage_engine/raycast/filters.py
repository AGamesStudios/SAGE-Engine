from typing import Callable
from ..objects.object import Object


def by_tag(tag: str) -> Callable[[Object], bool]:
    def _f(obj: Object) -> bool:
        return tag in obj.data.get("tags", [])
    return _f


def by_role(role: str) -> Callable[[Object], bool]:
    def _f(obj: Object) -> bool:
        return obj.has_role(role)
    return _f
