"""Role registration and built-in schemas with category support."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import json
from typing import Callable, Dict, Iterable, List, Mapping, Optional


@dataclass(frozen=True)
class Col:
    """Column description used inside a :class:`Category`."""

    name: str
    type: str
    default: object
    label: str | None = None
    min: float | None = None
    max: float | None = None


@dataclass(frozen=True)
class Category:
    """Logical group of columns."""

    name: str
    columns: List[Col]


@dataclass(frozen=True)
class RoleSchema:
    """Schema describing role categories."""

    name: str
    categories: List[Category]
    vtable: Optional[Iterable[str]] = None
    doc: str | None = None

    def to_json(self, store: Mapping[str, Mapping[str, List[object]]], row: int) -> dict:
        """Serialize data at *row* from *store* for this schema."""
        data: dict = {}
        for cat in self.categories:
            cat_data = {}
            for col in cat.columns:
                cat_data[col.name] = store[cat.name][col.name][row]
            data[cat.name] = cat_data
        return data

    def from_json(self, data: Mapping[str, Mapping[str, object]]) -> Mapping[str, object]:
        fields = {}
        for cat in self.categories:
            cat_data = data.get(cat.name, {})
            for col in cat.columns:
                fields[col.name] = cat_data.get(col.name, col.default)
        return fields


@dataclass
class RoleDefinition:
    name: str
    schema: RoleSchema
    vtable: Optional[Mapping[str, Callable]] = None


_registry: Dict[str, RoleDefinition] = {}


def _default_for(type_name: str) -> object:
    if type_name in {"float", "f32"}:
        return 0.0
    if type_name in {"int", "u32", "i32"}:
        return 0
    if type_name == "vec4":
        return [1.0, 1.0, 1.0, 1.0]
    return ""


def load_json_roles(directory: Path | None = None, docs_path: Path | None = None) -> None:
    """Load role definitions from JSON files and optionally generate docs."""
    from .. import core  # local import to avoid cycles
    from .. import scene, profiling

    if directory is None:
        directory = Path(__file__).resolve().parents[2] / "roles"
    if not directory.exists():
        return

    for path in sorted(directory.glob("*.role.json")):
        with open(path, "r", encoding="utf8") as fh:
            data = json.load(fh)
        cats = []
        for cat_name, fields in data.get("categories", {}).items():
            cols = [Col(fname, ftype, _default_for(ftype)) for fname, ftype in fields.items()]
            cats.append(Category(cat_name, cols))
        schema = RoleSchema(name=data["name"].lower(), categories=cats, vtable=data.get("phases"), doc=data.get("doc"))
        if schema.name not in _registry:
            register_role(schema)

        # Register simple update/draw phases counting calls
        if schema.vtable:
            for ph in schema.vtable:
                if ph == "update":
                    def updater(r=schema.name):
                        for obj in scene.scene.each_role(r):
                            profiling.profile.role_calls[r] = profiling.profile.role_calls.get(r, 0) + 1
                    core.register("update", updater)
                elif ph == "draw":
                    def drawer(r=schema.name):
                        for _ in scene.scene.each_role(r):
                            pass
                    core.register("draw", drawer)

    if docs_path:
        generate_docs(docs_path)


def generate_docs(path: Path) -> None:
    """Generate markdown documentation for all roles."""
    lines = ["# \U0001F4D8 \u0420\u043E\u043B\u0438", ""]
    for role in _registry.values():
        schema = role.schema
        lines.append(f"## \U0001F539 {schema.name}")
        if schema.doc:
            lines.append("")
            lines.append(schema.doc)
        lines.append("")
        lines.append("### \U0001F4E6 \u041A\u0430\u0442\u0435\u0433\u043E\u0440\u0438\u0438")
        for cat in schema.categories:
            cols = ", ".join(col.name for col in cat.columns)
            lines.append(f"- **{cat.name}**: {cols}")
        if schema.vtable:
            lines.append("")
            lines.append("### \U0001F527 \u0424\u0430\u0437\u044B")
            lines.append(", ".join(schema.vtable))
        lines.append("")
    path.write_text("\n".join(lines), encoding="utf8")


def register_role(schema: RoleSchema, vtable: Optional[Mapping[str, Callable]] = None) -> None:
    """Register a role definition."""

    if schema.name in _registry:
        raise ValueError(f"Role '{schema.name}' already registered")
    _registry[schema.name] = RoleDefinition(schema.name, schema, vtable)


def get_role(name: str) -> RoleDefinition:
    return _registry[name]


def registered_roles() -> Mapping[str, RoleDefinition]:
    return _registry


def roles_with(interface: str) -> List[str]:
    """Return names of roles that implement a given interface."""
    if interface not in INTERFACES:
        return []
    result = []
    for role_def in _registry.values():
        if any(cat.name == interface for cat in role_def.schema.categories):
            result.append(role_def.name)
    return result


from .interfaces import INTERFACES

# load roles from JSON definitions at import time if available
load_json_roles(docs_path=Path(__file__).resolve().parents[2] / "docs" / "roles.md")

