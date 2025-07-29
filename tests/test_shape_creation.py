def test_shape_creation():
    from examples.demo_shape_game import logic

    obj = logic.create("Shape", shape="circle", radius=10)
    assert obj is not None
