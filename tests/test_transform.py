import math

from sage_engine.transform import NodeTransform, Transform2D, prepare_world_all


def test_hierarchy_world_matrix():
    root = NodeTransform()
    root.transform.set_pos(10, 0)
    child = NodeTransform()
    root.add_child(child)
    child.transform.set_pos(5, 0)

    prepare_world_all(root)
    m_child = child.transform._m_world
    assert math.isclose(m_child[2], 15.0)


def test_dirty_flag_recomputes_once():
    node = NodeTransform()
    node.transform.set_pos(1, 2)
    prepare_world_all(node)
    first = node.transform._m_world[:]
    node.transform.set_pos(3, 4)
    prepare_world_all(node)
    second = node.transform._m_world
    assert first != second
