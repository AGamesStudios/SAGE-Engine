HEADER = b'SAGESFX' + bytes([1])


def encode(samples: bytes, channels: int = 1) -> bytes:
    return HEADER + bytes([channels]) + len(samples).to_bytes(4, 'little') + samples


def decode(data: bytes):
    if not data.startswith(HEADER):
        raise ValueError('invalid sagesfx header')
    channels = data[7]
    length = int.from_bytes(data[8:12], 'little')
    payload = data[12:12+length]
    return channels, payload
