"""Built-in effects implemented in pure Python."""
from __future__ import annotations

from ..api import register


def blur(buffer: bytearray, width: int, height: int, radius: int = 1) -> None:
    pitch = width * 4
    src = buffer[:]
    r = radius
    for y in range(height):
        for x in range(width):
            br = bg = bb = ba = count = 0
            for yy in range(max(0, y - r), min(height, y + r + 1)):
                for xx in range(max(0, x - r), min(width, x + r + 1)):
                    off = yy * pitch + xx * 4
                    bb += src[off]
                    bg += src[off + 1]
                    br += src[off + 2]
                    ba += src[off + 3]
                    count += 1
            off = y * pitch + x * 4
            buffer[off] = bb // count
            buffer[off + 1] = bg // count
            buffer[off + 2] = br // count
            buffer[off + 3] = ba // count


def glow(buffer: bytearray, width: int, height: int, radius: int = 4, color=(255, 255, 0)) -> None:
    # simple glow using blur on alpha and tinting
    blur(buffer, width, height, radius)
    pitch = width * 4
    for y in range(height):
        for x in range(width):
            off = y * pitch + x * 4
            buffer[off] = min(255, buffer[off] + color[2])
            buffer[off + 1] = min(255, buffer[off + 1] + color[1])
            buffer[off + 2] = min(255, buffer[off + 2] + color[0])


def ripple(buffer: bytearray, width: int, height: int, strength: int = 1) -> None:
    # naive ripple effect shifting rows
    pitch = width * 4
    src = buffer[:]
    for y in range(height):
        shift = (y % max(1, strength * 2)) - strength
        for x in range(width):
            xx = max(0, min(width - 1, x + shift))
            off_src = y * pitch + xx * 4
            off_dst = y * pitch + x * 4
            buffer[off_dst:off_dst+4] = src[off_src:off_src+4]


def color_matrix(buffer: bytearray, width: int, height: int, matrix=(1,0,0,0,
                                                                  0,1,0,0,
                                                                  0,0,1,0,
                                                                  0,0,0,1)) -> None:
    pitch = width * 4
    for y in range(height):
        for x in range(width):
            off = y * pitch + x * 4
            r = buffer[off + 2]
            g = buffer[off + 1]
            b = buffer[off]
            a = buffer[off + 3]
            nr = int(r*matrix[0] + g*matrix[1] + b*matrix[2] + a*matrix[3])
            ng = int(r*matrix[4] + g*matrix[5] + b*matrix[6] + a*matrix[7])
            nb = int(r*matrix[8] + g*matrix[9] + b*matrix[10]+ a*matrix[11])
            na = int(r*matrix[12]+ g*matrix[13]+ b*matrix[14]+ a*matrix[15])
            buffer[off + 2] = max(0, min(255, nr))
            buffer[off + 1] = max(0, min(255, ng))
            buffer[off] = max(0, min(255, nb))
            buffer[off + 3] = max(0, min(255, na))

# register builtins
register("blur", blur)
register("glow", glow)
register("ripple", ripple)
register("color_matrix", color_matrix)


def wave(buffer: bytearray, width: int, height: int, amplitude: int = 2, period: int = 16) -> None:
    import math
    pitch = width * 4
    src = buffer[:]
    for y in range(height):
        shift = int(math.sin(y / period * 2 * math.pi) * amplitude)
        for x in range(width):
            xx = (x + shift) % width
            off_src = y * pitch + xx * 4
            off_dst = y * pitch + x * 4
            buffer[off_dst:off_dst+4] = src[off_src:off_src+4]


def pixelate(buffer: bytearray, width: int, height: int, size: int = 4) -> None:
    pitch = width * 4
    src = buffer[:]
    for y in range(0, height, size):
        for x in range(0, width, size):
            off = y * pitch + x * 4
            color = src[off:off+4]
            for yy in range(y, min(height, y + size)):
                for xx in range(x, min(width, x + size)):
                    dst = yy * pitch + xx * 4
                    buffer[dst:dst+4] = color


def glitch(buffer: bytearray, width: int, height: int, intensity: int = 4) -> None:
    import random
    pitch = width * 4
    for _ in range(intensity * 10):
        x = random.randint(0, width - 1)
        y = random.randint(0, height - 1)
        off = y * pitch + x * 4
        for i in range(3):
            buffer[off + i] = random.randint(0, 255)


def fade(buffer: bytearray, width: int, height: int, alpha: float = 0.9) -> None:
    pitch = width * 4
    for y in range(height):
        for x in range(width):
            off = y * pitch + x * 4
            buffer[off + 3] = int(buffer[off + 3] * alpha)


def noise(buffer: bytearray, width: int, height: int, strength: int = 10) -> None:
    import random
    for i in range(0, len(buffer), 4):
        for j in range(3):
            val = buffer[i+j] + random.randint(-strength, strength)
            buffer[i+j] = max(0, min(255, val))


def outline(buffer: bytearray, width: int, height: int, color=(0,0,0)) -> None:
    pitch = width * 4
    src = buffer[:]
    for y in range(height):
        for x in range(width):
            off = y * pitch + x * 4
            a = src[off + 3]
            if a == 0:
                # check neighbors
                for dy in (-1,0,1):
                    for dx in (-1,0,1):
                        ny = y + dy
                        nx = x + dx
                        if 0 <= ny < height and 0 <= nx < width:
                            if src[ny*pitch+nx*4+3] > 0:
                                buffer[off] = color[2]
                                buffer[off+1] = color[1]
                                buffer[off+2] = color[0]
                                buffer[off+3] = 255
                                dy = dx = 2
                                break

# register new builtins
register("wave", wave)
register("pixelate", pixelate)
register("glitch", glitch)
register("fade", fade)
register("noise", noise)
register("outline", outline)

