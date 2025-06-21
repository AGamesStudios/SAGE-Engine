# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The latest version demonstrates more advanced shading with a baked global illumination
light map and a rotating cube.

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

This will open a window with a simple plane and a rotating cube lit by a single light.
The shaders include basic specular lighting and a placeholder light map that simulates
global illumination.
