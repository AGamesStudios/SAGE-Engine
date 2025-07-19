# SAGE Time

Provides simple timekeeping helpers.

```python
from sage_engine import time

time.boot()
time.mark()
# ... per frame
dt = time.get_delta()
print(time.get_time())
```

`time.scale` and `time.paused` can be tweaked to control progression.
