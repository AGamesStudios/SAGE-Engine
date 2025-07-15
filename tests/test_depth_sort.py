import sage_engine.sprites as sprites


def test_depth_sort():
    sprites.clear()
    a = sprites.Sprite(x=0.0, y=0.0, layer=1, z=0.0)
    b = sprites.Sprite(x=0.0, y=0.0, layer=0, z=0.5)
    sprites.add(a)
    sprites.add(b)
    inst = sprites.collect_instances()
    if hasattr(inst, "__len__") and not isinstance(inst, list):
        order = inst[:, -1]
    else:
        order = [row[-1] for row in inst]
    assert len(order) == 2
    assert order[0] == b.layer * 0.01 + b.z
    assert order[1] == a.layer * 0.01 + a.z
