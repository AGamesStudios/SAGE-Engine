from engine.chunk_delta import encode_ops, decode


def test_chunk_delta_roundtrip():
    ops = [b'a', b'bb']
    encoded = encode_ops(ops, feature_flags=1, crc_bloom=0xAA)
    count, flags, bloom, data = decode(encoded)
    assert count == 2
    assert flags == 1
    assert bloom == 0xAA
    assert data == b''.join(ops)
