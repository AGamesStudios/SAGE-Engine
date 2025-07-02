"""Utility functions for texture and icon handling."""

from __future__ import annotations

import ctypes
from PIL import Image
from OpenGL.GL import (
    glGenTextures,
    glBindTexture,
    glTexParameteri,
    glTexImage2D,
    glDeleteTextures,
    GL_TEXTURE_2D,
    GL_TEXTURE_MIN_FILTER,
    GL_TEXTURE_MAG_FILTER,
    GL_LINEAR,
    GL_NEAREST,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
)

from engine.utils.log import logger


def get_blank_texture(renderer, smooth: bool = True) -> int:
    """Return a 1x1 white texture."""
    attr = "_blank_texture" if smooth else "_blank_nearest_texture"
    tex = getattr(renderer, attr, None)
    if tex is None:
        data = b"\xff\xff\xff\xff"
        tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex)
        filt = GL_LINEAR if smooth else GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt)
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            1,
            1,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data,
        )
        setattr(renderer, attr, tex)
    return tex


def get_texture(renderer, obj) -> int:
    if obj.image is None:
        return get_blank_texture(renderer, getattr(obj, "smooth", True))
    key = (id(obj.image), bool(getattr(obj, "smooth", True)))
    tex = renderer.textures.get(key)
    if tex:
        return tex
    img = obj.image.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    data = img.tobytes()
    tex_id = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, tex_id)
    filt = GL_LINEAR if getattr(obj, "smooth", True) else GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt)
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        img.width,
        img.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data,
    )
    renderer.textures[key] = tex_id
    return tex_id


def get_icon_texture(renderer, name: str) -> int:
    try:
        from engine.icons import ICON_DIR, ICON_THEME
    except Exception:  # pragma: no cover - optional
        try:
            from sage_editor.icons import ICON_DIR, ICON_THEME
        except Exception:  # pragma: no cover - no icons available
            return get_blank_texture(renderer)
    key = f"{ICON_THEME}/{name}"
    tex = renderer._icon_cache.get(key)
    if tex:
        return tex
    path = ICON_DIR / ICON_THEME / name
    if not path.is_file():
        alt = "white" if ICON_THEME == "black" else "black"
        alt_path = ICON_DIR / alt / name
        if alt_path.is_file():
            path = alt_path
        else:
            root_path = ICON_DIR / name
            if root_path.is_file():
                path = root_path
            else:
                return get_blank_texture(renderer)
    try:
        img = Image.open(path).convert("RGBA")
    except Exception:
        logger.exception("Failed to load icon %s", path)
        return get_blank_texture(renderer)
    img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    data = img.tobytes()
    tex = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, tex)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        img.width,
        img.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data,
    )
    renderer._icon_cache[key] = tex
    return tex


def clear_icon_cache(renderer) -> None:
    if not renderer._icon_cache:
        return
    try:
        arr = (ctypes.c_uint * len(renderer._icon_cache))(*renderer._icon_cache.values())
        renderer.widget.makeCurrent()
        glDeleteTextures(len(renderer._icon_cache), arr)
        renderer.widget.doneCurrent()
    except Exception:
        logger.exception("Failed to delete icon textures")
    renderer._icon_cache.clear()
    renderer._missing_icons.clear()
