# Engine Basics

The engine provides a lightweight core for games. Scenes contain objects which update and render every frame.

The core modules are written in pure Python with optional renderers and input
backends loaded via plugins. Most features work without external dependencies so
projects remain portable and easy to extend.

Thanks to this approach the engine also runs on **PyPy 3.10+**. Using PyPy can
speed up CPU heavy logic thanks to its Just-In-Time compiler while keeping the
same Python API.

The event system optionally updates in parallel. Enable ``async_events`` or
``asyncio_events`` on :class:`Engine` to process event logic concurrently.
When integrating with other :mod:`asyncio` code use ``await Engine.run_async()``
to run the engine without blocking the calling loop. High level helpers
``run_project_async`` and ``run_scene_async`` mirror the synchronous API.
Each function returns the :class:`~engine.game_window.GameWindow` when the Qt
backend is available so the window can be controlled programmatically.
Extension hooks can be coroutines and ``Object.update`` may yield a coroutine
when using ``run_async``.

The ``ResourceManager`` includes asynchronous variants of its import and load
methods so resources can be processed without blocking the event loop.

Each ``Engine`` maintains a sprite cache for loaded images. Adjust the
``image_cache_limit`` via ``EngineSettings`` or replace it entirely with
``engine.entities.game_object.set_sprite_cache`` when you need finer control.
Engine settings also accept an ``Environment`` object holding global options
such as the background colour. ``engine.environment`` can be updated at runtime
to change these properties.

Rotation angles wrap by default once they exceed ``360`` degrees. Configure the
limit with ``EngineSettings.max_angle`` and use ``Object.rotate(da, around_bbox=True)``
to spin objects around their bounding box centre instead of their pivot.
Set ``EngineSettings.rotate_bbox`` to ``True`` if objects should rotate around
their bounding box centre by default when calling ``Object.rotate``.
When ``around_bbox`` is omitted, :class:`GameObject.rotate` uses this setting to
decide whether the bounding box or pivot should stay fixed.

Complex shapes can be built by combining multiple meshes with
``engine.mesh_utils.union_meshes`` before assigning them to an object's
``mesh`` attribute. ``union_meshes`` only supports the ``negatives`` argument
when :mod:`shapely` is installed; otherwise it raises ``ImportError``.
Alternatively use
``engine.mesh_utils.difference_meshes`` to remove one mesh from another.
Custom shapes can be created with
``engine.mesh_utils.create_polygon_mesh`` by providing a list of vertices.
The resulting :class:`~engine.mesh_utils.Mesh` can be transformed, joined
with ``union_meshes`` or subtracted using ``difference_meshes``. Boolean
operations triangulate polygons so interior holes are preserved. Simple
polygons created with ``create_polygon_mesh`` remain as n-gons without
triangulation. Complex shapes can produce many triangles and slow rendering; see
[limitations](limitations.md). Use
``Mesh.apply_matrix`` or ``Transform2D.apply_matrix`` together with
:func:`engine.core.math2d.make_transform` to apply translations, scaling and
rotations in one step.

Install the ``physics`` extra to enable a lightweight wrapper around
`pymunk`. Create a :class:`~engine.physics.PhysicsWorld`, add shapes for your
objects and attach :class:`~engine.physics.PhysicsExtension` to the engine to
step the world each frame.

Materials describe the appearance of a sprite. A :class:`~engine.entities.object.Material`
defines a base ``color`` (RGBA tuple), optional ``texture`` path and ``opacity``
value. When creating a :class:`~engine.entities.game_object.GameObject`, any missing
``color``, ``image_path`` or ``alpha`` fields are taken from the material so roles
can provide consistent defaults.

## Object Roles

Objects carry a ``role`` string instead of a fixed class type. Register new
roles with :func:`engine.entities.object.register_role` to provide default
logic, materials or metadata. Built-in roles ``empty``, ``shape``, ``sprite`` and
``camera`` can be used directly with :func:`engine.entities.object.create_role`.
```python
from sage_engine.entities.object import register_role, create_role

def spinner(obj, dt):
    obj.rotate(90 * dt)

register_role('spinner', logic=[spinner])
spin_obj = create_role('spinner')
```
