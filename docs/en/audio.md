# Audio Support

SAGE Engine plays sound effects through the ``AudioManager`` class. Install the
``audio`` extra to enable it:

```bash
pip install .[audio]
```

Load and play sounds from your resources folder:

```python
from engine.audio import AudioManager

am = AudioManager()
am.play('sounds/jump.wav')
```

You can also load a `.sageaudio` descriptor containing metadata:

```python
snd = am.load_sound("jump.sageaudio")
```

If the descriptor specifies a `volume` key the sound is automatically scaled.

## Music playback

``AudioManager`` exposes helper methods to control background music:

```python
am.load_music('music/theme.ogg')
am.play_music(loops=-1)    # loop forever
am.set_music_volume(0.5)
```

Use ``stop_music()`` and ``pause_music()`` to control playback.
