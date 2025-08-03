# Transform

The transform module provides lightweight 2D transformation utilities.  A
:class:`~sage_engine.transform.Transform2D` stores position, rotation, scale and
origin and lazily builds 3×3 matrices.  :class:`~sage_engine.transform.NodeTransform`
links transforms into hierarchies allowing world matrices to be computed on
demand.  Matrices are cached and recomputed only when inputs change.

Before rendering a frame the helper :func:`~sage_engine.transform.prepare_world_all`
can be used to update a whole tree of transforms.
