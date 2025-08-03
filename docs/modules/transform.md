# Transform

The transform module provides lightweight 2D transformation utilities.  A
:class:`~sage_engine.transform.Transform2D` stores position, rotation, scale and
origin and lazily builds 3×3 matrices.  All transformable objects implement
the :class:`~sage_engine.transform.BaseTransform` interface which exposes
``world_matrix`` and ``world_aabb`` helpers.  Two concrete implementations are
provided:

* :class:`~sage_engine.transform.RectangleTransform` – standalone transform with
  a local rectangle.
* :class:`~sage_engine.transform.NodeTransform` – extends the base interface
  with parent/children links allowing world matrices to be computed for
  hierarchical scenes.

Matrices are cached and recomputed only when inputs change.

Before rendering a frame the helper :func:`~sage_engine.transform.prepare_world_all`
can be used to update a whole tree of transforms.
It also populates per-frame counters stored in
:mod:`sage_engine.transform.stats` and mirrored to
:mod:`sage_engine.render.stats` such as ``transform_nodes_updated`` and
``transform_visible_objects``.

## Spaces

``Space`` enumerates the main coordinate systems:

* ``LOCAL`` – coordinates relative to the object.
* ``WORLD`` – global world units (Y up).
* ``SCREEN`` – pixel space (Y down).

Conversion helpers like :func:`~sage_engine.transform.world_to_screen` and
:func:`~sage_engine.transform.screen_to_world` move coordinates between these
spaces.

The module also offers bounding box and culling helpers.  Bounds can be queried
in local, world or screen space via :func:`~sage_engine.transform.get_local_aabb`,
:func:`~sage_engine.transform.get_world_aabb` and
:func:`~sage_engine.transform.get_screen_bounds`.  Visibility checks are handled
by :class:`~sage_engine.transform.TransformCuller` and
:func:`~sage_engine.transform.intersects_screen`.  Every transform exposes
``global_position`` and ``global_scale`` helpers as well as an
``is_visible(camera)`` convenience wrapper.

Per-frame counters live in :mod:`sage_engine.transform.stats` and can be
inspected with ``sage info transform`` which now reports total, visible and
culled objects along with the traversal depth.

Transforms can be serialized with
:func:`~sage_engine.transform.serialize_transform` and later restored with
:func:`~sage_engine.transform.apply_transform` which is useful for deterministic
tests and tooling.

``pixel_snap`` and ``snap_rect`` provide pixel perfect positioning taking camera
zoom into account.
