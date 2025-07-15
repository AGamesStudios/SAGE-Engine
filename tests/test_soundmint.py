from sage_engine.build import convert_audio, sha1


def test_convert_audio(tmp_path):
    source = tmp_path / "beep.ogg"
    source.write_bytes(b"data")
    cache = tmp_path / "cache"
    dest = convert_audio(str(source), str(cache))
    assert dest.exists()
    expected = cache / f"{sha1(source)}.mp3"
    assert dest == expected
    dest_time = dest.stat().st_mtime
    # second call should not rewrite
    dest2 = convert_audio(str(source), str(cache))
    assert dest2.stat().st_mtime == dest_time
