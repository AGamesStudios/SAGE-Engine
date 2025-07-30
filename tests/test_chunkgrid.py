from sage_engine.render.optimizer.chunks import ChunkGrid


def test_chunkgrid_clear():
    grid = ChunkGrid(chunk_size=8)
    grid.add('obj', (0, 0, 4, 4))
    assert grid.chunks
    grid.clear()
    assert not grid.chunks
