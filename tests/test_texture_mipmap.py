from sage_engine.texture import Texture

def test_texture_generate_mipmap(tmp_path):
    from sage_engine.format import sageimg
    p = tmp_path / "tex.sageimg"
    p.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    tex = Texture()
    tex.load(str(p), generate_mipmap=True)
    assert tex.mipmaps is not None
