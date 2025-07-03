from __future__ import annotations

from dataclasses import dataclass, field
from typing import Callable, List, Any, Optional, TYPE_CHECKING
import itertools

from ..utils.log import logger

if TYPE_CHECKING:
    from ..core.scenes.scene import Scene
from ..core.math2d import (
    make_transform,
    transform_point,
    angle_to_quat,
    normalize_angle,
)

_ID_GEN = itertools.count()

def next_id() -> int:
    """Return a unique object id."""
    return next(_ID_GEN)

@dataclass(slots=True)
class Transform2D:
    """Simple 2D transform container."""

    x: float = 0.0
    y: float = 0.0
    scale_x: float = 1.0
    scale_y: float = 1.0
    angle: float = 0.0
    pivot_x: float = 0.0
    pivot_y: float = 0.0
    rotation: tuple[float, float, float, float] = field(init=False, repr=False)

    def __post_init__(self) -> None:
        self.angle = normalize_angle(self.angle)
        self.rotation = angle_to_quat(self.angle)

    def __setattr__(self, name, value):
        if name == "angle":
            value = normalize_angle(value)
            object.__setattr__(self, "rotation", angle_to_quat(value))
        object.__setattr__(self, name, value)

    def matrix(self) -> List[float]:
        """Return a 3x3 column-major transform matrix."""
        return make_transform(
            self.x,
            self.y,
            self.scale_x,
            self.scale_y,
            self.angle,
            self.pivot_x,
            self.pivot_y,
        )


@dataclass(slots=True)
class Material:
    """Simple material used by sprite roles."""

    color: tuple[int, int, int, int] = (255, 255, 255, 255)
    texture: str | None = None
    opacity: float = 1.0
    blend: str = "alpha"


@dataclass(slots=True)
class Variable:
    """Typed variable stored on an :class:`Object`."""

    type: str = "any"
    value: Any = None
    public: bool = True


@dataclass(slots=True)
class Object:
    """Engine object with a role, transform and attached logic."""

    role: str
    name: Optional[str] = None
    transform: Transform2D = field(default_factory=Transform2D)
    logic: List[Callable[["Object", float], Any]] = field(default_factory=list)
    material: Material | None = None
    metadata: dict = field(default_factory=dict)
    variables: dict[str, "Variable"] = field(default_factory=dict)
    visible: bool = True
    group: str | None = None
    id: int = field(init=False)
    parent: Optional["Object"] = field(default=None, repr=False)
    children: List["Object"] = field(default_factory=list, repr=False)
    scene: "Scene | None" = field(init=False, default=None, repr=False)

    # --- common properties ---
    @property
    def position(self) -> tuple[float, float]:
        """Return the local position as ``(x, y)``."""
        return self.transform.x, self.transform.y

    @position.setter
    def position(self, value: tuple[float, float]) -> None:
        self.transform.x, self.transform.y = value

    @property
    def rotation(self) -> float:
        """Return the object rotation angle in degrees."""
        return self.transform.angle

    @rotation.setter
    def rotation(self, angle: float) -> None:
        self.transform.angle = angle

    @property
    def scale(self) -> tuple[float, float]:
        """Return the object's scale as ``(sx, sy)``."""
        return self.transform.scale_x, self.transform.scale_y

    @scale.setter
    def scale(self, value: tuple[float, float]) -> None:
        sx, sy = value
        self.transform.scale_x = sx
        self.transform.scale_y = sy

    def __setattr__(self, name, value):
        if name == "group":
            old = getattr(self, "group", None)
            object.__setattr__(self, name, value)
            scene = getattr(self, "scene", None)
            if scene is not None and old != value:
                scene.update_object_group(self, old)
            return
        object.__setattr__(self, name, value)

    def __post_init__(self) -> None:
        self.id = next_id()
        if self.name is None:
            self.name = self.role
        if self.role == "sprite" and self.material is None:
            self.material = Material()
        if self.role == "camera":
            self.metadata.setdefault("width", 640)
            self.metadata.setdefault("height", 480)
            self.metadata.setdefault("active", False)

    def update(self, dt: float) -> None:
        """Call logic callbacks sequentially."""
        for func in list(self.logic):
            try:
                func(self, dt)
            except Exception:
                logger.exception("Logic error in %s", self.role)

    def add_logic(self, func: Callable[["Object", float], Any]) -> None:
        """Attach a logic callback if not already present."""
        if func not in self.logic:
            self.logic.append(func)

    def remove_logic(self, func: Callable[["Object", float], Any]) -> None:
        """Remove a previously attached logic callback."""
        if func in self.logic:
            self.logic.remove(func)

    # --- variable helpers ---
    def add_variable(self, name: str, value: Any = None, typ: str = "any", public: bool = True) -> None:
        """Create or update a typed variable.

        ``public`` indicates whether the variable should be visible to the
        scene event system when building events.
        """
        self.variables[name] = Variable(type=typ, value=value, public=public)
        if hasattr(self, "public_vars"):
            if public:
                self.public_vars.add(name)
            else:
                self.public_vars.discard(name)

    def get_variable(self, name: str, default: Any = None) -> Any:
        var = self.variables.get(name)
        return var.value if var is not None else default

    def set_variable(self, name: str, value: Any, public: bool | None = None) -> None:
        """Update ``name`` with ``value``.

        If ``public`` is not ``None`` the visibility of the variable is
        adjusted accordingly.
        """
        if name in self.variables:
            self.variables[name].value = value
            if public is not None:
                self.variables[name].public = public
        else:
            self.variables[name] = Variable(value=value, public=public if public is not None else True)
        if hasattr(self, "public_vars") and public is not None:
            if public:
                self.public_vars.add(name)
            else:
                self.public_vars.discard(name)

    # --- transformation helpers ---
    def move(self, dx: float, dy: float) -> None:
        """Translate the object by ``dx`` and ``dy``."""
        self.transform.x += dx
        self.transform.y += dy

    def rotate(self, da: float) -> None:
        """Rotate the object by ``da`` degrees."""
        self.transform.angle += da

    def set_scale(self, sx: float, sy: Optional[float] = None) -> None:
        """Set the object's scale."""
        if sy is None:
            sy = sx
        self.transform.scale_x = sx
        self.transform.scale_y = sy

    # --- hierarchy management ---
    def add_child(self, child: "Object") -> None:
        """Attach ``child`` to this object."""
        if child.parent is self:
            return
        if child.parent is not None:
            child.parent.remove_child(child)
        child.parent = self
        self.children.append(child)

    def remove_child(self, child: "Object") -> None:
        """Detach ``child`` from this object."""
        if child in self.children:
            self.children.remove(child)
            child.parent = None

    def find_child(self, name: str) -> Optional["Object"]:
        """Recursively search for a child by name."""
        for ch in self.children:
            if ch.name == name:
                return ch
            found = ch.find_child(name)
            if found is not None:
                return found
        return None

    # --- world space helpers ---
    def world_matrix(self) -> List[float]:
        """Return the transform matrix with parents applied."""
        mat = self.transform.matrix()
        if self.parent is not None:
            pmat = self.parent.world_matrix()
            # 3x3 matrix multiply (column-major)
            mat = [
                pmat[0] * mat[0] + pmat[3] * mat[1] + pmat[6] * mat[2],
                pmat[1] * mat[0] + pmat[4] * mat[1] + pmat[7] * mat[2],
                pmat[2] * mat[0] + pmat[5] * mat[1] + pmat[8] * mat[2],
                pmat[0] * mat[3] + pmat[3] * mat[4] + pmat[6] * mat[5],
                pmat[1] * mat[3] + pmat[4] * mat[4] + pmat[7] * mat[5],
                pmat[2] * mat[3] + pmat[5] * mat[4] + pmat[8] * mat[5],
                pmat[0] * mat[6] + pmat[3] * mat[7] + pmat[6] * mat[8],
                pmat[1] * mat[6] + pmat[4] * mat[7] + pmat[7] * mat[8],
                pmat[2] * mat[6] + pmat[5] * mat[7] + pmat[8] * mat[8],
            ]
        return mat

    def world_position(self) -> tuple[float, float]:
        """Return the world position of this object."""
        if self.parent is None:
            return self.transform.x, self.transform.y
        mat = self.parent.world_matrix()
        return transform_point(mat, self.transform.x, self.transform.y)

    def destroy(self) -> None:
        """Detach from parent and clear children."""
        if self.parent is not None:
            self.parent.remove_child(self)
        for ch in list(self.children):
            ch.destroy()


ROLE_DEFAULTS = {
    "empty": {},
    "sprite": {"material": Material()},
    "camera": {"metadata": {"width": 640, "height": 480, "active": False}},
}


def create_role(
    role: str,
    *,
    name: str | None = None,
    transform: Transform2D | None = None,
    logic: list[Callable[[Object, float], Any]] | None = None,
    material: Material | None = None,
    metadata: dict | None = None,
) -> Object:
    """Return an :class:`Object` with defaults for ``role``."""
    defaults = ROLE_DEFAULTS.get(role, {})
    if material is None:
        material = defaults.get("material")
    meta = dict(defaults.get("metadata", {}))
    if metadata:
        meta.update(metadata)
    return Object(
        role=role,
        name=name,
        transform=transform or Transform2D(),
        logic=list(logic) if logic else [],
        material=material,
        metadata=meta,
    )

