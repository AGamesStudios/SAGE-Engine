import lzma
import struct
from typing import Iterable, Tuple

from .capabilities import missing_caps_from_flags
from .utils.log import logger

__all__ = [
    "CHUNK_FORMAT_MAJOR",
    "CHUNK_FORMAT_MINOR",
    "CHUNK_HEADER",
    "CHUNK_HEADER_SIZE",
    "encode_ops",
    "decode",
]

CHUNK_FORMAT_MAJOR = 1
CHUNK_FORMAT_MINOR = 0

CHUNK_HEADER = "<HHHHQ"
CHUNK_HEADER_SIZE = struct.calcsize(CHUNK_HEADER)


def encode_ops(
    ops: Iterable[bytes],
    feature_flags: int = 0,
    crc_bloom: int = 0,
    *,
    format_major: int = CHUNK_FORMAT_MAJOR,
    format_minor: int = CHUNK_FORMAT_MINOR,
) -> bytes:
    """Return LZMA-compressed chunk delta from operations."""
    ops_list = list(ops)
    body = b"".join(ops_list)
    header = struct.pack(
        CHUNK_HEADER,
        format_major,
        format_minor,
        len(ops_list),
        feature_flags,
        crc_bloom,
    )
    return lzma.compress(header + body)


def decode(data: bytes) -> Tuple[int, int, int, int, int, bytes]:
    """Decode a compressed chunk.

    Returns ``(major, minor, count, flags, bloom, body)``.
    """
    payload = lzma.decompress(data)
    major, minor, count, flags, bloom = struct.unpack(
        CHUNK_HEADER, payload[:CHUNK_HEADER_SIZE]
    )
    if major != CHUNK_FORMAT_MAJOR:
        raise ValueError(f"unsupported chunk format {major}.{minor}")
    if minor != CHUNK_FORMAT_MINOR:
        logger.warning(
            "Chunk minor version %s differs from runtime %s",
            minor,
            CHUNK_FORMAT_MINOR,
        )
    missing = missing_caps_from_flags(flags)
    if missing:
        logger.warning(
            "Chunk requires unsupported capabilities: %s",
            ", ".join(missing),
        )
    body = payload[CHUNK_HEADER_SIZE:]
    return major, minor, count, flags, bloom, body
