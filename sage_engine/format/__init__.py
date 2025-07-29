"""Minimal SAGE binary format implementation."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import IntEnum
from pathlib import Path
from typing import Any, Tuple


import struct
import yaml


class TLVType(IntEnum):
    U8 = 0x01
    U16 = 0x02
    U32 = 0x03
    F32 = 0x04
    STR = 0x05
    ARR = 0x06
    MAP = 0x07
    FLAG = 0x08
    REF = 0x09


def _encode_length(n: int) -> bytes:
    """Encode a length value using 1 or 3 bytes."""
    if n < 0xFF:
        return bytes([n])
    return b"\xFF" + n.to_bytes(2, "big")


def _decode_length(data: bytes, off: int) -> Tuple[int, int]:
    first = data[off]
    off += 1
    if first == 0xFF:
        length = int.from_bytes(data[off:off + 2], "big")
        off += 2
    else:
        length = first
    return length, off


def _encode_value(value: Any) -> bytes:
    if isinstance(value, bool):
        return bytes([TLVType.FLAG]) + _encode_length(1) + (b"\x01" if value else b"\x00")
    if isinstance(value, int):
        if 0 <= value <= 0xFF:
            return bytes([TLVType.U8]) + _encode_length(1) + value.to_bytes(1, "big")
        if 0 <= value <= 0xFFFF:
            return bytes([TLVType.U16]) + _encode_length(2) + value.to_bytes(2, "big")
        if 0 <= value <= 0xFFFFFFFF:
            return bytes([TLVType.U32]) + _encode_length(4) + value.to_bytes(4, "big")
        raise ValueError("int out of range")
    if isinstance(value, float):
        return bytes([TLVType.F32]) + _encode_length(4) + struct.pack("<f", value)
    if isinstance(value, str):
        b = value.encode("utf8")
        return bytes([TLVType.STR]) + _encode_length(len(b)) + b
    if isinstance(value, list):
        encoded = b"".join(_encode_value(v) for v in value)
        return bytes([TLVType.ARR]) + _encode_length(len(encoded)) + encoded
    if isinstance(value, dict):
        items = b"".join(_encode_value(k) + _encode_value(v) for k, v in value.items())
        return bytes([TLVType.MAP]) + _encode_length(len(items)) + items
    raise TypeError(f"Unsupported type: {type(value)!r}")


def _decode_value(data: bytes, off: int) -> Tuple[Any, int]:
    t = TLVType(data[off])
    off += 1
    length, off = _decode_length(data, off)
    if t == TLVType.U8:
        v = data[off]
        off += 1
        return v, off
    if t == TLVType.U16:
        v = int.from_bytes(data[off:off + 2], "big")
        off += 2
        return v, off
    if t == TLVType.U32:
        v = int.from_bytes(data[off:off + 4], "big")
        off += 4
        return v, off
    if t == TLVType.F32:
        v = struct.unpack("<f", data[off:off + 4])[0]
        off += 4
        return v, off
    if t == TLVType.STR:
        v = data[off:off + length].decode("utf8")
        off += length
        return v, off
    if t == TLVType.ARR:
        end = off + length
        arr = []
        while off < end:
            item, off = _decode_value(data, off)
            arr.append(item)
        return arr, off
    if t == TLVType.MAP:
        end = off + length
        res = {}
        while off < end:
            key, off = _decode_value(data, off)
            val, off = _decode_value(data, off)
            res[key] = val
        return res, off
    if t == TLVType.FLAG:
        v = data[off] != 0
        off += 1
        return v, off
    if t == TLVType.REF:
        v = data[off:off + length].decode("utf8")
        off += length
        return {"ref": v}, off
    raise ValueError(f"unknown TLV type {t}")


HEADER = b"SAGE"
VERSION = 1


@dataclass
class SAGECompiler:
    """Compile YAML/JSON into binary SAGE format."""

    def compile(self, src: Path, dst: Path) -> None:
        data = yaml.safe_load(src.read_text(encoding="utf8"))
        encoded = HEADER + bytes([VERSION]) + _encode_value(data)
        dst.write_bytes(encoded)


@dataclass
class SAGEDecompiler:
    """Convert binary SAGE format back into Python data."""

    def decompile(self, path: Path) -> Any:
        buf = path.read_bytes()
        if not buf.startswith(HEADER):
            raise ValueError("invalid header")
        _, off = HEADER, 4
        version = buf[4]
        if version != VERSION:
            raise ValueError("unsupported version")
        value, _ = _decode_value(buf, 5)
        return value


@dataclass
class SAGESchemaSystem:
    """Very small schema registry for validation."""

    schemas: dict[str, dict[str, type]] = field(default_factory=dict)

    def register(self, name: str, schema: dict[str, type]) -> None:
        self.schemas[name] = dict(schema)

    def validate(self, name: str, data: dict) -> None:
        schema = self.schemas.get(name)
        if not schema:
            return
        for key, typ in schema.items():
            if key not in data:
                raise ValueError(f"missing field: {key}")
            if not isinstance(data[key], typ):
                raise TypeError(f"{key} must be {typ.__name__}")


__all__ = [
    "SAGECompiler",
    "SAGEDecompiler",
    "SAGESchemaSystem",
    "load_sage_file",
    "pack_directory",
    "HEADER",
    "VERSION",
]

from .loader import load_sage_file
from .packer import pack_directory

