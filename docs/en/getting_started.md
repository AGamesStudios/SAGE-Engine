# Getting Started

This quick guide shows how to draw a sprite using the engine and explains the
basic storage APIs.

```bash
pip install -r requirements.txt
pip install -e .
python examples/templates/hello_sprite_py/hello_sprite.py
```

## Hello Sprite
You can use the ready-made example at
`examples/templates/hello_sprite_py/hello_sprite.py` or create
`hello_sprite.py` yourself with the following code:
```python
from sage_engine import Engine, GameObject, ResourceManager

res = ResourceManager()
tex = res.load_image('examples/Resources/logo.png')
obj = GameObject(texture=tex, x=320, y=240)

Engine(scene=[obj]).run()
```
Running the script opens a window and displays the sprite.

## NanoTree API
`NanoTree` maps a file into memory and stores nodes with 1 KiB slices:
```python
from sage_engine import NanoTree

with NanoTree('state.bin') as tree:
    node = tree.add_node(crc=0x12345678)
    slice_idx = tree.alloc_slice()
    tree.set_node_slice(node, slice_idx)
    tree.slice(slice_idx).write(b'data')
```
Nodes and slices persist so the tree can be reopened later.

## SmartSlice API
`SmartSliceAllocator` manages a circular queue of 1 KiB blocks:
```python
from sage_engine import SmartSliceAllocator

alloc = SmartSliceAllocator('pool.bin', 128)
mark = alloc.mark(snapshot_id=0)
block = alloc.alloc_slice(type_id=1, count=4)
alloc.free_mark(mark)
```
Use `mark()` to store a snapshot position and `free_mark()` to release
older blocks in bulk.

## Working from PyCharm
1. Create a virtual environment with `python -m venv .venv`.
2. Build the Rust extension using `maturin develop`. Set `SAGE_BUILD=debug` to
   disable optimisation.
3. Run `main.py` from the IDE.

```
$ python -q
>>> import nano_core
>>> nano_core.merge_chunk_delta(b'a', b'b')
b'ab'
```
