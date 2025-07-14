# Sprite Rendering Pipeline

SAGE draws sprites through the `SpritePass` class. All sprite images are packed into an atlas
using `tools/pack_atlas.py` which produces `atlas.png` and `atlas.json`.
`atlas.json` stores the coordinates of each source image. During `sage build` the
utility is invoked automatically if an `assets/` directory contains PNG files.

`SpritePass` uploads per-instance data from a `SmartSliceAllocator` and issues a
single draw call with a bindless texture array when WGPU is available. The
OpenGL fallback binds textures one by one but uses the same atlas layout.

Performance stats are written to Chrome Trace when profiling is enabled. Look for
"SpritePass CPU" and "GPUSubmit" events.

Sprite roles require the fields `x`, `y`, `width`, `height` and `atlas_index`
to select the region in the atlas.
