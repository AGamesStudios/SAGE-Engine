# Custom shaders and materials

SAGE Engine allows sprites to use user-defined GLSL programs. A `ShaderProgram`
wraps the vertex and fragment source. Load shaders from files and create a
`Material` to hold the program and any uniform values:

```python
from sage_engine.render.shader import load
from sage_engine.render.material import Material

shader = load("tint", "tint.vert", "tint.frag")
mat = Material(shader, {"u_tint": (1.0, 0.0, 0.0)})
```

Assign the material to a sprite before drawing. The engine groups sprites by
material and issues one draw call per group. The OpenGL backend switches
programs and updates uniforms automatically.

See `examples/custom_shader_sprite` for a minimal demo applying a grayscale
fragment shader.
