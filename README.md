# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The engine demonstrates baked global illumination using a light map. It now uses a
fixed camera so the scene is easier to view and enables multi-sample anti aliasing
for smoother edges.

Shadows are rendered with a depth map and filtered using a high quality 7x7 PCF kernel for
even softer edges. The engine verifies that the shadow framebuffer is complete
and uses a 32‑bit depth texture so shadows appear reliably. A small polygon
offset is applied when rendering the depth map to avoid acne artifacts. Each
object now has its own model matrix so the plane no longer rotates with the cube
and the shadows line up correctly. Lighting combines a directional light with a
point light and stronger ambient illumination. A screen‑space ambient occlusion
(SSAO) pass further darkens corners for more realism. The shaders combine the
baked light map with dynamic lighting and shadowing to approximate global
illumination.

## Requirements

- Python 3
- PyOpenGL
- numpy
- A valid OpenGL context (GLUT). On Linux install `freeglut3-dev` with your
  package manager. Running the example also requires access to an X11 display
  so the window can be created.

Install the requirements with:

```bash
pip install PyOpenGL PyOpenGL_accelerate numpy
```

Then run the example:

```bash
python main.py
```

This demo opens a window with a plane and a rotating cube lit by a directional
light and a point light. The camera is fixed in place so the scene stays in
view. Shadows come from a depth map filtered with a 7×7 PCF kernel and a
screen-space ambient occlusion pass darkens creases using a G-buffer built in
view space. The G-buffer textures clamp to the screen edges so the SSAO result
is free of border artifacts. The SSAO pass now includes a blur stage to reduce
noise. Multi-sample anti aliasing and baked global illumination make the final
image smoother and brighter.

## Quality Modes

The renderer now supports a simplified *low* quality mode to help it run on
weaker PCs. Run `python main.py low` and the engine will:

- Use a smaller shadow map
- Disable SSAO
- Render with fewer depth samples

Running `python main.py` without arguments keeps the original high quality
settings.
