HEADER = b'SAGEIMG' + bytes([1])


def encode(pixels: bytes, width: int, height: int) -> bytes:
    return HEADER + width.to_bytes(2, 'little') + height.to_bytes(2, 'little') + pixels


def decode(data: bytes):
    if not data.startswith(HEADER):
        raise ValueError('invalid sageimg header')
    width = int.from_bytes(data[7:9], 'little')
    height = int.from_bytes(data[9:11], 'little')
    pixels = data[11:]
    return width, height, pixels
