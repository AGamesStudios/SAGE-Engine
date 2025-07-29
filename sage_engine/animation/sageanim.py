"""Encode and decode simple .sageanim files."""

from __future__ import annotations

from typing import List, Tuple

HEADER = b'SAGEANIM' + bytes([1])


def encode(frames: List[int], durations: List[int], loop: int | bool, events: List[str]) -> bytes:
    assert len(frames) == len(durations) == len(events)
    buf = bytearray(HEADER)
    buf += len(frames).to_bytes(2, 'little')
    for idx, dur, ev in zip(frames, durations, events):
        buf += idx.to_bytes(2, 'little')
        buf += dur.to_bytes(2, 'little')
        ev_b = ev.encode('utf8')
        buf += len(ev_b).to_bytes(1, 'little')
        buf += ev_b
    if loop is True:
        loop_val = 0xFFFF
    elif loop is False:
        loop_val = 0
    else:
        loop_val = int(loop)
    buf += loop_val.to_bytes(2, 'little')
    return bytes(buf)


def decode(data: bytes) -> Tuple[List[int], List[int], int | bool, List[str]]:
    if not data.startswith(HEADER):
        raise ValueError('invalid sageanim header')
    off = len(HEADER)
    count = int.from_bytes(data[off:off+2], 'little'); off += 2
    frames: List[int] = []
    durations: List[int] = []
    events: List[str] = []
    for _ in range(count):
        idx = int.from_bytes(data[off:off+2], 'little'); off += 2
        dur = int.from_bytes(data[off:off+2], 'little'); off += 2
        ev_len = data[off]; off += 1
        ev = data[off:off+ev_len].decode('utf8'); off += ev_len
        frames.append(idx)
        durations.append(dur)
        events.append(ev)
    loop_val = int.from_bytes(data[off:off+2], 'little')
    if loop_val == 0xFFFF:
        loop: int | bool = True
    elif loop_val == 0:
        loop = False
    else:
        loop = loop_val
    return frames, durations, loop, events
