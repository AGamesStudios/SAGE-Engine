from sage_engine.render import soa

def test_batch_add():
    batch = soa.SpriteBatch(2)
    batch.add(1,2,3,4,0xFFFFFFFF)
    batch.add(5,6,7,8,0xFFFFFF00)
    assert batch.count == 2
    assert batch.x[1] == 5
    batch.add(9,9,9,9,0)
    assert batch.count == 2  # capacity limit
