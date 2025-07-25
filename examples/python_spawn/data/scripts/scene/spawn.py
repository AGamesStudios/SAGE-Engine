from sage_engine.logic_api import on_ready, create_object
import random
import math


def spawn() -> None:
    for i in range(3):
        x = math.cos(i) * 50
        y = random.randint(-40, 40)
        create_object(f"box{i}", "Sprite", {"image": "box.png", "x": x, "y": y})


on_ready(spawn)
