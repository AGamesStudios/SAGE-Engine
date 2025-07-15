# Writing your own render backend

SAGE Engine discovers render backends via the `sage_render` entry point group. A backend
implements `sage_engine.render.base.RenderBackend` and registers itself in
`pyproject.toml`:

```toml
[project.entry-points.sage_render]
custom = "my_package.backend:MyBackend"
```

A minimal backend must implement `create_device`, `begin_frame`,
`draw_sprites`, `end_frame` and `resize`. Optional helpers such as
`draw_lines` can be left empty. Recent versions also use
`create_texture` and `set_camera` to upload atlas images and update the
view matrix.

```python
from sage_engine.render.base import RenderBackend

class VulkanBackend(RenderBackend):
    def create_device(self, width: int, height: int) -> None:
        ...

    def begin_frame(self) -> None:
        ...

    def draw_sprites(self, instances) -> None:
        ...

    def end_frame(self) -> None:
        ...

    def resize(self, width: int, height: int) -> None:
        ...

    def create_texture(self, image) -> int:
        ...

    def set_camera(self, matrix):
        ...
```

During development you can select the backend via the CLI:

```bash
sage run --render opengl your_game.py
```

Images for the atlas can be loaded with Pillow:

```python
from PIL import Image
atlas_id, uv = backend.create_texture(Image.open("hero.png"))
```

Instances passed to `draw_sprites` are arrays of floats with the
following layout:

```
[x, y, sx, sy, rot, atlas_id, u0, v0, u1, v1, blend, r, g, b, a, depth]
```

`blend` is `0.0` for standard alpha blending and `1.0` for premultiplied
alpha.

## Depth handling

Sprites have integer `layer` and float `z` values. The engine sorts
instances by `(layer, z)` before rendering and passes a combined
`depth` float to the backend. Your vertex shader should assign this
value to `gl_Position.z` so that sprites in higher layers appear on
top. Depth is calculated as `layer * 0.01 + z`, so avoid using hundreds
of consecutive layers to keep precision reasonable.
