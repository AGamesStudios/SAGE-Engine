import os
from engine.chrono_patch import ChronoPatchTree


def test_chrono_patch(tmp_path):
    path = tmp_path / "state.bin"
    tree = ChronoPatchTree(str(path), size=4096)
    tree.snapshot(0, b"hello")
    tree.close()

    new_tree = ChronoPatchTree(str(path), size=4096)
    new_tree.replay()
    assert bytes(new_tree.slice(0).view[:5]) == b"hello"
    new_tree.close()
    assert os.path.exists(str(path) + ".log")
