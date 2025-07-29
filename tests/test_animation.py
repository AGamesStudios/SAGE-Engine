from sage_engine.animation import AnimationPlayer, sageanim
from sage_engine.sprite import sprite


def test_simple_loop(tmp_path):
    img = sprite.Sprite(1, 1, b"\x00")
    data = sageanim.encode([0, 1], [5, 5], True, ["", ""])
    anim_path = tmp_path / "a.sageanim"
    anim_path.write_bytes(data)

    player = AnimationPlayer(str(anim_path), img)
    player.play()
    player.update(5)
    assert player.frame_index == 1
    player.update(5)
    assert player.frame_index == 0  # looped
