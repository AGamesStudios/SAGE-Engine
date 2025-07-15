# Tilemap quick start

`TileLayer.from_tmx()` loads CSV encoded layers from Tiled maps. Parallax factors
can be specified via `parallaxx`/`parallaxy` attributes or a `parallax` property
on the layer. Drawing offsets are derived from the camera position:

```python
from sage_engine import tilemap
layer = tilemap.TileLayer.from_tmx('level.tmx', layer=0)
cam_x = 100
x, y = layer.draw_offset(cam_x, 0)
```

Tiles with the custom property `collidable=true` create static bodies in a
`physics.World` when passed to `from_tmx`:

```python
from sage_engine import physics, tilemap

world = physics.World()
tilemap.TileLayer.from_tmx('level.tmx', layer=1, world=world)
```

Set `one_way=true` on a tile to create a platform that only collides when
approached from above:

```python
tilemap.TileLayer.from_tmx('level.tmx', layer=2, world=world)
```

## Parallax

Background layers with a parallax value less than `1` scroll more slowly than
the camera, creating a depth effect. Use X‑Ray debug mode to visualize the
collider outlines and a faint tile grid.
