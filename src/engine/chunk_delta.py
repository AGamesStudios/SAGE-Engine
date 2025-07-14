import lzma
import struct
from typing import Iterable, Tuple

from .capabilities import missing_caps_from_flags
from .utils.log import logger

__all__ = ["CHUNK_HEADER", "CHUNK_HEADER_SIZE", "encode_ops", "decode"]

CHUNK_HEADER = "<HHQ"
CHUNK_HEADER_SIZE = struct.calcsize(CHUNK_HEADER)


def encode_ops(ops: Iterable[bytes], feature_flags: int = 0, crc_bloom: int = 0) -> bytes:
    """Return LZMA-compressed chunk delta from operations."""
    ops_list = list(ops)
    body = b"".join(ops_list)
    header = struct.pack(CHUNK_HEADER, len(ops_list), feature_flags, crc_bloom)
    return lzma.compress(header + body)


def decode(data: bytes) -> Tuple[int, int, int, bytes]:
    """Decode a compressed chunk and return count, flags, bloom and body."""
    payload = lzma.decompress(data)
    count, flags, bloom = struct.unpack(CHUNK_HEADER, payload[:CHUNK_HEADER_SIZE])
    missing = missing_caps_from_flags(flags)
    if missing:
        logger.warning("Chunk requires unsupported capabilities: %s", ", ".join(missing))
    body = payload[CHUNK_HEADER_SIZE:]
    return count, flags, bloom, body
