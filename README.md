# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The engine demonstrates baked global illumination using a light map. It now uses a
fixed camera so the scene is easier to view and enables multi-sample anti aliasing
for smoother edges.

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
The camera is fixed in place so the cube and plane remain in view. The shaders
apply a baked global illumination map and gamma correction. Multi-sample anti
aliasing is enabled for smoother rendering.
