from examples.demo_shape_game import logic, enemy


def test_enemy_despawn_timing():
    logic.init_game()
    e = enemy.Enemy.create(100, logic.HEIGHT - 10)
    logic.enemies.append(e)
    # Update once; enemy still on screen
    logic.update(0.1)
    assert e in logic.enemies
    # Run updates until beyond screen + height
    for _ in range(40):
        logic.update(0.1)
        if e not in logic.enemies:
            break
    assert e not in logic.enemies

