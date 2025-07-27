"""Camera role schema using categories."""

from . import Category, Col, RoleSchema


CAMERA_SCHEMA = RoleSchema(
    name="camera",
    categories=[
        Category(
            "transform",
            [
                Col("x", "f32", 0.0),
                Col("y", "f32", 0.0),
            ],
        ),
        Category(
            "camera",
            [
                Col("zoom", "f32", 1.0),
            ],
        ),
    ],
)
