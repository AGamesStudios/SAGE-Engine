from sage_engine import chunk_delta
from sage_engine.chunk_delta import encode_ops, decode


def test_chunk_delta_roundtrip():
    ops = [b'a', b'bb']
    encoded = encode_ops(ops, feature_flags=1, crc_bloom=0xAA)
    major, minor, count, flags, bloom, data = decode(encoded)
    assert major == chunk_delta.CHUNK_FORMAT_MAJOR
    assert minor == chunk_delta.CHUNK_FORMAT_MINOR
    assert count == 2
    assert flags == 1
    assert bloom == 0xAA
    assert data == b"".join(ops)


def test_chunk_delta_major_mismatch():
    encoded = encode_ops([b"x"], format_major=chunk_delta.CHUNK_FORMAT_MAJOR + 1)
    try:
        decode(encoded)
    except ValueError:
        assert True
    else:  # pragma: no cover - should not happen
        assert False
