# RenderFabric

`RenderFabric` initialises a WGPU device and falls back to OpenGL 3.3 when WGPU is unavailable. The `SpritePass` class batches sprites using a bindless atlas and instancing. The goal is about 500 FPS when drawing 1,000 sprites at 720p on an Intel HD 610 GPU.
