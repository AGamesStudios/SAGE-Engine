# ChunkΔ Codec

`encode_ops()` packs a sequence of operations into a chunk using a small header
``(count, featureFlags, crcBloom)`` and LZMA compression. `decode()` reverses the
process, returning the header fields and the raw operation bytes.
