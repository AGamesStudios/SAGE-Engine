from sage_engine.transform import NodeTransform, Camera2D
from sage_engine.transform.core import prepare_world_all, TransformCuller
from sage_engine.transform import stats as tstats


def test_culling_counts():
    root = NodeTransform()
    visible = NodeTransform()
    hidden = NodeTransform()
    hidden.transform.set_pos(1000, 1000)
    root.add_child(visible)
    root.add_child(hidden)
    tstats.reset_frame()
    prepare_world_all(root)
    cam = Camera2D(pos=(0, 0), viewport_px=(100, 100), zoom=1.0)
    culler = TransformCuller(cam)
    vis = culler.collect(root)
    assert hidden not in vis
    assert tstats.stats["culling_rejected"] >= 1
