# Audio quick start

The audio subsystem loads short sound effects using `sage_audio.play`. Playback
supports looping, stereo panning and pitch control. Handles returned by
`play()` can be stopped later. The global ``mixer`` lists active tracks and
exposes a master volume slider. Audio playback relies on ``simpleaudio`` (and
optionally ``pydub``) which are listed in ``requirements.txt``.

```python
from sage_engine import audio

handle = audio.play("sounds/laser.wav", loop=True, pan=-0.5, pitch=1.2)
print(audio.mixer.active())  # list of playing handles
handle.stop()
```

## SoundMint: automatic converter
Install the optional ``audio_conv`` extras to enable on-the-fly conversion
via FFmpeg. The ``convert`` helper writes
to ``build/audio_cache`` and skips re-encoding when the source file hasn't
changed.

```python
from sage_engine.soundmint import convert

dest = convert("sounds/beep.wav", "ogg")
```

You can also run ``sage build --target-fmt mp3`` to convert all OGG files to
MP3 during bundling.
