# Sprite Animation

The :class:`engine.animation.Animation` class plays sequences of :class:`engine.animation.Frame` objects.
Each frame stores an image path and a duration in seconds. New fields provide more
control over playback:

* ``speed`` – multiplier applied to frame durations (``1.0`` is normal speed)
* ``playing`` – set ``False`` to pause the animation
* ``reverse`` – when ``True`` frames play in reverse order

``Animation.play()``, ``pause()`` and ``stop()`` control playback. ``stop()`` also
resets the current frame index.
