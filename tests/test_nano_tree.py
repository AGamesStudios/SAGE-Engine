from engine.nano_tree import NanoTree


def test_nano_tree(tmp_path):
    path = tmp_path / "tree.bin"
    with NanoTree(str(path)) as tree:
        idx = tree.add_node(crc=0x1)
        slice_idx = tree.alloc_slice()
        tree.set_node_slice(idx, slice_idx)
        tree.slice(slice_idx).write(b"hello")

    with NanoTree(str(path)) as reopened:
        node = reopened.get_node(0)
        assert node.slice_index == 0
        data = reopened.slice(node.slice_index).read(5)
        assert data == b"hello"
