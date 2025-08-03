# Transform

The transform module provides lightweight 2D transformation utilities.  A
:class:`~sage_engine.transform.Transform2D` stores position, rotation, scale and
origin and lazily builds 3×3 matrices.  :class:`~sage_engine.transform.NodeTransform`
links transforms into hierarchies allowing world matrices to be computed on
demand.  Matrices are cached and recomputed only when inputs change.

Before rendering a frame the helper :func:`~sage_engine.transform.prepare_world_all`
can be used to update a whole tree of transforms.

The module also offers simple bounding box and culling helpers.  Each
``NodeTransform`` may store a local rectangle describing its bounds.  The
functions :func:`~sage_engine.transform.get_world_aabb` and
:func:`~sage_engine.transform.collect_visible` transform these boxes to world
space and allow frustum culling against a ``Camera2D`` viewport.  Per-frame
counters are exposed in :mod:`sage_engine.render.stats`.

Transforms can be serialized with
:func:`~sage_engine.transform.serialize_transform` and later restored with
:func:`~sage_engine.transform.apply_transform` which is useful for deterministic
tests and tooling.
