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

Rotation angles wrap by default once they exceed ``360`` degrees. Configure the
limit with ``EngineSettings.max_angle`` and use ``Object.rotate(da, around_bbox=True)``
to spin objects around their bounding box centre instead of their pivot.
Set ``EngineSettings.rotate_bbox`` to ``True`` if objects should rotate around
their bounding box centre by default when calling ``Object.rotate``.
When ``around_bbox`` is omitted, :class:`GameObject.rotate` uses this setting to
decide whether the bounding box or pivot should stay fixed.

Complex shapes can be built by combining multiple meshes with
``engine.mesh_utils.union_meshes`` before assigning them to an object's
``mesh`` attribute. When :mod:`shapely` is available, ``union_meshes`` can also
subtract ``negatives`` from the positive shapes. Alternatively use
``engine.mesh_utils.difference_meshes`` to remove one mesh from another.

Materials describe the appearance of a sprite. A :class:`~engine.entities.object.Material`
defines a base ``color`` (RGBA tuple), optional ``texture`` path and ``opacity``
value. When creating a :class:`~engine.entities.game_object.GameObject`, any missing
``color``, ``image_path`` or ``alpha`` fields are taken from the material so roles
can provide consistent defaults.

## Object Roles

Objects carry a ``role`` string instead of a fixed class type. Register new
roles with :func:`engine.entities.object.register_role` to provide default
logic, materials or metadata. Built-in roles ``empty``, ``sprite`` and
``camera`` can be used directly with :func:`engine.entities.object.create_role`.
```python
from engine.entities.object import register_role, create_role

def spinner(obj, dt):
    obj.rotate(90 * dt)

register_role('spinner', logic=[spinner])
spin_obj = create_role('spinner')
```
