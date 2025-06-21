# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The engine demonstrates baked global illumination using a light map. It now uses a
fixed camera so the scene is easier to view and enables multi-sample anti aliasing
for smoother edges.

 Shadows are rendered with a depth map and filtered using a high quality 7x7 PCF kernel for
 even softer edges. The engine verifies that the shadow framebuffer is complete
 and uses a 32‑bit depth texture so shadows appear reliably. Lighting now
 combines a directional light with a point light and stronger ambient illumination. A
 screen‑space ambient occlusion (SSAO) pass further darkens corners for more
 realism. The shaders combine the baked light map with dynamic lighting and
 shadowing to approximate global illumination.

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

This will open a window with a plane and a rotating cube lit by a directional
light and a point light. The camera is fixed in place so the cube and plane
remain in view. The
shaders apply a baked global illumination map, soft shadow mapping with PCF
filtering (now using a 7x7 kernel), screen-space ambient occlusion and gamma
correction. Multi-sample anti aliasing is enabled for smoother rendering. A
slightly stronger ambient term keeps objects visible even in shadowed areas.
