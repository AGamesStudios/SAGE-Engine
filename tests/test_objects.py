from sage_engine import objects, sprites


def test_role_registration():
    objects.register_role("enemy", ["orc", "goblin"])
    assert set(objects.categories_for("enemy")) == {"goblin", "orc"}


def test_collect_groups():
    sp1 = sprites.Sprite(0, 0)
    sp2 = sprites.Sprite(1, 0)
    o1 = objects.Object(sp1, role="enemy", category="orc", layer=1)
    o2 = objects.Object(sp2, role="enemy", category="goblin", layer=1)
    objects.add(o1)
    objects.add(o2)
    groups = objects.collect_groups()
    roles = {r for r, c, m, inst in groups}
    cats = {c for r, c, m, inst in groups if r == "enemy"}
    assert roles == {"enemy"}
    assert cats == {"orc", "goblin"}
    objects.clear()
