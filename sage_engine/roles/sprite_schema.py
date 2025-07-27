"""Example role schema for sprites using categories."""

from . import Category, Col, RoleSchema


SPRITE_SCHEMA = RoleSchema(
    name="sprite",
    categories=[
        Category(
            "transform",
            [
                Col("x", "f32", 0.0),
                Col("y", "f32", 0.0),
            ],
        ),
        Category(
            "sprite",
            [
                Col("texture", "str", ""),
                Col("tint", "u32", 0xFFFFFFFF),
            ],
        ),
    ],
)
