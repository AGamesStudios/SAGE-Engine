from engine.nano_merge import merge_ops


def test_merge_ops_sort_and_match():
    ops = [(2, b'b'), (1, b'a'), (3, b'c')]
    python_result = merge_ops(ops, simd=False)
    simd_result = merge_ops(ops, simd=True)
    assert python_result == b"abc"
    assert simd_result == python_result
