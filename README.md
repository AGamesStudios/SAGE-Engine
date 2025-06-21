# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The latest version demonstrates baked global illumination using a light map and
adds a simple orbiting camera.

## Requirements

- Python 3
- PyOpenGL
- A valid OpenGL context (GLUT)

Install the requirements with:

```bash
pip install PyOpenGL PyOpenGL_accelerate
```

Then run the example:

```bash
python main.py
```

This will open a window with a plane and a rotating cube lit by a single light.
The camera slowly circles the scene so you can watch the lighting change from
different angles. The shaders include basic specular lighting and a placeholder
light map that simulates indirect lighting.
