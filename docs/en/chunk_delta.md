# ChunkΔ Codec

`encode_ops()` packs operations into a chunk with the header
``(format_major, format_minor, count, featureFlags, crcBloom)`` and applies LZMA
compression. `decode()` returns these fields plus the raw operation bytes. If the
major version differs from the runtime the chunk fails to load. A minor mismatch
emits a warning and conversion may occur.
