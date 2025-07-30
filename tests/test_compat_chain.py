from sage_engine import compat

def test_compat_chain():
    data_v0 = {"x": 1}

    def m0_to_1(d):
        d["x"] += 1
        d["schema_version"] = 1
        return d

    def m1_to_2(d):
        d["x"] += 1
        d["schema_version"] = 2
        return d

    compat.register("demo", m0_to_1, 0, 1)
    compat.register("demo", m1_to_2, 1, 2)
    ver, out = compat.migrate("demo", 0, 2, dict(data_v0))
    assert ver == 2
    assert out["x"] == 3
