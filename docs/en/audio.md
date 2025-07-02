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
