# Sprite Rendering

Sprites use a texture atlas system so that many images can be drawn in a single
call. The resource manager uploads textures to the first atlas that has enough
space and creates new atlases only when required. Every loaded texture is cached
by its image data so the same picture is never uploaded twice even if it appears
under different file names.

Call `resources.manager.stats()` to inspect the current atlases:

```python
info = resources.manager.stats()
print(info["atlases"], "atlases", info["textures"], "textures")
```

The returned dictionary lists all loaded texture names and the remaining pixel
capacity of each atlas. When bundling or running `sage serve` the engine prints a
summary like:

```
🧵 3 atlases created (2048×2048), 38 textures loaded
```

New atlases are only allocated once existing ones have no room for the next
image.
