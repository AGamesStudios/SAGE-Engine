import os
from sage_engine.chrono_patch import ChronoPatchTree


def test_chrono_patch(tmp_path):
    path = tmp_path / "state.bin"
    with ChronoPatchTree(str(path), size=4096) as tree:
        tree.snapshot(0, b"hello")

    with ChronoPatchTree(str(path), size=4096) as new_tree:
        new_tree.replay()
        assert new_tree.slice(0).read(5) == b"hello"
        patches = list(new_tree.iter_patches())
    assert patches == [(0, b"hello")]
    assert os.path.exists(str(path) + ".log")
