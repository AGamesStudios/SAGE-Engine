# SAGE Render

The SAGE Render subsystem batches Sprite objects by their image so large scenes
can be drawn with only a few draw calls. Objects are sorted by the `layer`
property before batching. UI elements and cameras are also processed during this
step.

The function `render_scene(objects)` returns a list describing the draw calls
for testing purposes. Each `SpriteBatch` entry notes the image and how many
sprites were batched together.
