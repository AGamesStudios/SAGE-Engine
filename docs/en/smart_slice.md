# SmartSlice Allocator

`SmartSliceAllocator` manages a circular pool of 1 KiB slices inside a memory mapped file.
Use `alloc_slice(type_id, count)` to reserve contiguous blocks and `mark(snapshot_id)` to
record a position. Later `free_mark(snapshot_id)` releases all slices allocated up to that
mark, allowing bulk cleanup after snapshots.
