# Audio quick start

The audio subsystem loads short sound effects using `sage_audio.play`. Playback
supports looping, stereo panning and pitch control. A simple mixer panel lets
you adjust track volume during runtime.

```python
from sage_engine import audio

audio.play("sounds/laser.wav", loop=True, pan=-0.5, pitch=1.2)
```
