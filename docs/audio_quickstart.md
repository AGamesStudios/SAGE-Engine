# Audio quick start

The audio subsystem loads short sound effects using `sage_audio.play`. Playback
supports looping, stereo panning and pitch control. Handles returned by
`play()` can be stopped later. The global ``mixer`` lists active tracks and
exposes a master volume slider.

```python
from sage_engine import audio

handle = audio.play("sounds/laser.wav", loop=True, pan=-0.5, pitch=1.2)
print(audio.mixer.active())  # list of playing handles
handle.stop()
```
