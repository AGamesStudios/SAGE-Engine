# Coordinate Conversion

The coordinate helpers translate points between world and screen spaces.  A
small :class:`Camera2D <sage_engine.transform.convert.Camera2D>` data class
defines the view.  Functions such as
:func:`~sage_engine.transform.world_to_screen` and
:func:`~sage_engine.transform.screen_to_world` handle the math.
