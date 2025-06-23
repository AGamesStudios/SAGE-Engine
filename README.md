# SAGE Engine Example

This repository contains a minimal example of a Python renderer built with OpenGL.
The engine demonstrates baked global illumination using a light map. The camera
now orbits around the scene so the objects remain visible from all sides while
multi-sample anti aliasing smooths the image.

Shadows are rendered with a depth map and filtered using a Poisson‑disk based PCSS
(percentage‑closer soft shadows) algorithm. A blocker search first estimates the
average occluder depth and then uses that to widen the filtering radius so edges
grow softer with distance. Random rotation of the Poisson kernel reduces banding
while keeping the filter lightweight. Shadows now use an orthographic projection
so the entire scene fits inside the shadow map. The depth texture is filtered
with `GL_LINEAR` and the engine verifies that the shadow framebuffer is complete
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
- numba and scipy (optional for JIT acceleration)
- A valid OpenGL context (GLUT). On Linux install `freeglut3-dev` with your
  package manager. Running the example also requires access to an X11 display
  so the window can be created.

Install the requirements with:

```bash
pip install PyOpenGL PyOpenGL_accelerate numpy numba scipy
```

Then run the example:

```bash
python main.py
```

This demo opens a window with a plane and a cube lit by a directional light and
a point light. The objects are static while the camera slowly circles them. Shadows
come from a depth map filtered with a Poisson‑disk based PCSS filter and a
screen-space ambient occlusion pass darkens creases using a G-buffer built in
view space. The G-buffer textures clamp to the screen edges so the SSAO result
is free of border artifacts. The SSAO pass now includes a blur stage to reduce
noise. Multi-sample anti aliasing and baked global illumination make the final
image smoother and brighter. Lighting uses classic Phong specular highlights.

Internal math like matrix calculation and SSAO sample generation can be JIT
compiled with [Numba](https://numba.pydata.org/) when both Numba and SciPy are
installed. If SciPy is missing the code automatically falls back to plain
NumPy. The update loop uses a timer so the scene redraws roughly 60 times per
second even on older PCs.

Framebuffers and textures are recreated when the window is resized so shadows
and ambient occlusion remain crisp regardless of resolution.

## Quality Modes

The renderer now supports a simplified *low* quality mode to help it run on
weaker PCs. Run `python main.py low` and the engine will:

- Use a smaller shadow map
- Disable SSAO
- Render with fewer depth samples
- Use half as many shadow samples for filtering

Running `python main.py` without arguments keeps the original high quality
settings.
