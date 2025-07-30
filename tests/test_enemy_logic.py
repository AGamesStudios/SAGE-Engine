from examples.demo_shape_game import logic, enemy


def test_enemy_despawn_timing():
    logic.init_game()
    e = enemy.Enemy.create(100, logic.HEIGHT - 10)
    logic.enemies.append(e)
    # enemy is partially visible, should not despawn yet
    for _ in range(5):
        logic.update(0.1)
        assert e in logic.enemies
    # continue updating until it leaves the screen
    for _ in range(60):
        logic.update(0.1)
        if e not in logic.enemies:
            break
    assert e not in logic.enemies

