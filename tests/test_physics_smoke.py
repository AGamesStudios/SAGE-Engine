from sage_engine import physics


def test_sensor_callback():
    world = physics.World(gravity=10.0)
    ball = world.create_box(y=10)
    sensor_triggered = []
    sensor = world.create_box(behaviour="sensor")
    sensor.on_contact = lambda b: sensor_triggered.append(b)
    for _ in range(60):
        world.step(0.1)
    assert sensor_triggered
    assert ball.y >= 0
