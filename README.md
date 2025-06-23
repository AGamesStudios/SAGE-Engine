# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The engine demonstrates baked global illumination using a light map. The camera
now orbits around the scene so the objects remain visible from all sides while
multi-sample anti aliasing smooths the image.

Shadows are rendered with a depth map and filtered using a Poisson disk PCF pattern for
softer edges without visible banding. The engine verifies that the shadow framebuffer is complete
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

This demo opens a window with a plane and a cube lit by a directional light and
a point light. The objects are static while the camera slowly circles them. Shadows
come from a depth map filtered with a Poisson disk PCF kernel and a
screen-space ambient occlusion pass darkens creases using a G-buffer built in
view space. The G-buffer textures clamp to the screen edges so the SSAO result
is free of border artifacts. The SSAO pass now includes a blur stage to reduce
noise. Multi-sample anti aliasing and baked global illumination make the final
image smoother and brighter. Lighting uses classic Phong specular highlights.

Framebuffers and textures are recreated when the window is resized so shadows
and ambient occlusion remain crisp regardless of resolution.

## Quality Modes

The renderer now supports a simplified *low* quality mode to help it run on
weaker PCs. Run `python main.py low` and the engine will:

- Use a smaller shadow map
- Disable SSAO
- Render with fewer depth samples

Running `python main.py` without arguments keeps the original high quality
settings.
