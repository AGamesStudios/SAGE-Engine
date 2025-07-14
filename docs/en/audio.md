# Audio Support

SAGE Engine ships with ``sage_audio`` built on the ``miniaudio`` library. Install
the ``audio`` extra to enable it:

```bash
pip install .[audio]
```

Play a sound from your resources folder:

```python
from engine.audio import play

snd = play('sounds/jump.ogg', loop=False, gain=0.8, pan=-0.3)
snd.set_pitch(1.2)
```

You can also load a `.sageaudio` descriptor containing metadata:

```python
snd = am.load_sound("jump.sageaudio")
```

If the descriptor specifies a `volume` key the sound is automatically scaled.

## Music playback

The returned ``Sound`` handle allows you to stop playback with ``stop()`` or
change pitch with ``set_pitch()``.
