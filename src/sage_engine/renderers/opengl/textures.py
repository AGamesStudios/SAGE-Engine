"""Utility functions for texture and icon handling."""


from PIL import Image  # type: ignore[import-not-found]
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
)  # type: ignore[import-not-found]


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


def unload_texture(renderer, obj) -> None:
    """Remove ``obj``'s texture from the cache."""
    key = (id(obj.image), bool(getattr(obj, "smooth", True)))
    tex = renderer.textures.pop(key, None)
    if tex:
        glDeleteTextures([tex])


