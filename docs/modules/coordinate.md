# Coordinate Conversion

The coordinate helpers translate points between world and screen spaces.  A
small :class:`Camera2D <sage_engine.transform.convert.Camera2D>` data class
defines the view.  Functions such as
:func:`~sage_engine.transform.world_to_screen` and
:func:`~sage_engine.transform.screen_to_world` handle the math.  Additional
helpers like :func:`~sage_engine.transform.screen_rect_to_world` convert
selection rectangles, while :func:`~sage_engine.transform.pixel_snap` rounds
coordinates to the nearest integer pixel which is useful for pixel perfect
rendering.
Matrix data is prepared each frame via
``transform.prepare_world_all`` so conversions stay in sync with the
rendered scene.
