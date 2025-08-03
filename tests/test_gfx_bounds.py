from sage_engine.gfx.runtime import GraphicRuntime


def test_blend_pixel_out_of_bounds():
    rt = GraphicRuntime()
    rt.init(4, 4)
    # should not raise even if coordinates are outside
    rt._blend_pixel(-1, 0, 255, 0, 0, 255)
    rt._blend_pixel(10, 10, 255, 0, 0, 255)
