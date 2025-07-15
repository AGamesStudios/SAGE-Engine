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
tex_id = backend.create_texture(Image.open("hero.png"))
```
