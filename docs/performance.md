# 📘 Performance

Approximate draw cost on reference devices:

| Device | 720p draw cost |
|-------|---------------|
| Desktop i5 | ~4 ms |
| Laptop low-end | ~7 ms |
| Raspberry Pi | ~9 ms |

`dynamic_resolution` temporarily lowers resolution when draw cost exceeds 8 ms.

