# NanoMerge

`merge_ops()` sorts operations by CRC and writes the data in one pass. When `numpy` is available the function uses vectorised sorting for a significant speed boost compared to the pure Python fallback.
