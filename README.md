# SAGE Render

This repository contains a minimal example of a Python renderer built with OpenGL.
The engine demonstrates baked global illumination using a light map. The camera
now orbits around the scene so the objects remain visible from all sides while
multi-sample anti aliasing smooths the image.

Shadows are rendered with an exponential variance shadow map that stores four
depth moments. A Poisson disk kernel with random rotation filters the map and
uses a Chebyshev bound on the exponential moments to estimate occlusion.
Storing four moments greatly reduces light bleeding compared to the old two
moment approach. Shadows still use an orthographic projection so the entire
scene fits inside the map. The texture is linearly filtered and the framebuffer
is verified for completeness. A small polygon offset avoids acne artifacts.
The EVSM exponent was lowered so the depth moments stay within floating point
range, preventing shadows from turning completely dark.
Each object now has its own model matrix so the plane no longer rotates with the cube and the shadows line up correctly. Lighting combines a directional light with a
point light and stronger ambient illumination. A screen‑space ambient occlusion
(SSAO) pass further darkens corners for more realism. The shaders combine the
baked light map with dynamic lighting and shadowing to approximate global
illumination. The fragment shader uses a simple microfacet model with
metallic and roughness parameters for more realistic lighting.

## SAGE Render Technology

SAGE Render groups the rendering code into a reusable Python module. It
implements exponential variance shadow maps with Poisson filtering, a blur-enhanced SSAO pass and an
orbiting camera. The engine exposes an `Engine` class and a `main()` helper so
the demo can be started with `python -m sage_render` or by importing the module
in your own projects.

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
python -m sage_render
# or just
python main.py
```

This demo opens a window with a plane and a cube lit by a directional light and
a point light. The objects are static while the camera slowly circles them. Shadows
come from an exponential variance shadow map filtered with a Poisson disk kernel and a
screen-space ambient occlusion pass darkens creases using a G-buffer built in
view space. The G-buffer textures clamp to the screen edges so the SSAO result
is free of border artifacts. The SSAO pass now includes a blur stage to reduce
noise and clamps the output so overly dark pixels are avoided. Multi-sample anti aliasing and baked global illumination make the final
image smoother and brighter. Lighting now relies on a simple physically based
microfacet shader with metallic and roughness controls instead of the old Phong
highlights.

Internal math like matrix calculation and SSAO sample generation can be JIT
compiled with [Numba](https://numba.pydata.org/) when both Numba and SciPy are
installed. If SciPy is missing the code automatically falls back to plain
NumPy. The update loop uses a timer so the scene redraws roughly 60 times per
second even on older PCs.

Framebuffers and textures are recreated when the window is resized so shadows
and ambient occlusion remain crisp regardless of resolution.

## Quality Modes

The renderer now supports a simplified *low* quality mode to help it run on
weaker PCs. Run `python -m sage_render low` (or `python main.py low`) and the
engine will:

- Use a smaller shadow map
- Disable SSAO
- Render with fewer depth samples
- Use half as many shadow samples for filtering

Running `python -m sage_render` without arguments keeps the original high
quality settings.

### Debug Output

Add the `debug` argument to enable OpenGL debug messages:

```bash
python -m sage_render debug
```

You can also switch quality at runtime by calling `set_quality('low')` or
`set_quality('high')` on the `Engine` instance.
