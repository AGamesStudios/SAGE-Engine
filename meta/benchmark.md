# Benchmark Results

Tests were run on a 4-core Linux VM using `python3.11`.

| Scenario | Objects | FPS | Frame Time (ms) | Notes |
|---------|---------|-----|----------------|------|
| baseline | 100 | 850 | 1.1 | No effects |
| heavy sprites | 1000 | 220 | 4.5 | CPU backend |
| stress | 10000 | 35 | 28 | GC noticeable |

Memory usage peaked at 60 MB. No significant leaks were observed during 5 minute runs.
