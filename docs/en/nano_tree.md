# NanoTree

NanoTree is a minimal memory mapped tree used by the engine. The file stores a
header with the node and slice counts followed by fixed size nodes and
1 KiB slices. Each node stores a CRC32 path hash, its parent index and the slice
index for data.

Use :class:`engine.nano_tree.NanoTree` to allocate nodes and slices:

```python
from sage_engine import NanoTree

with NanoTree('state.bin') as tree:
    node = tree.add_node(crc=0x12345678)
    slice_idx = tree.alloc_slice()
    tree.set_node_slice(node, slice_idx)
    tree.slice(slice_idx).write(b'data')
```

Nodes and slices persist in the mapped file so the tree can be reopened later.
