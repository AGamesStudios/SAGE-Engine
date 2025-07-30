import random
from sage_engine.objects import Object, runtime
try:
    from .shape_role import Shape
except ImportError:  # fallback when run as script
    from shape_role import Shape


def create(role: str, **fields) -> Object:
    """Create an object with the given role and add it to the store."""
    obj = Object(id=f"obj_{len(runtime.store.objects)}")
    if role == "Shape":
        obj.add_role("Shape", Shape(**fields))
    runtime.store.add_object(obj)
    return obj


try:
    from .player import Player
    from .bullet import Bullet
    from .enemy import Enemy
except ImportError:
    from player import Player
    from bullet import Bullet
    from enemy import Enemy
from sage_engine.input import Input

WIDTH = 640
HEIGHT = 360

player: Player | None = None
bullets: list[Bullet] = []
enemies: list[Enemy] = []
score = 0
_spawn = 0.0


def init_game() -> None:
    global player, bullets, enemies, score, _spawn
    runtime.store.objects.clear()
    runtime.store.by_role.clear()
    runtime.store.by_world.clear()
    player = Player.create(WIDTH // 2 - 15, HEIGHT - 30)
    bullets.clear()
    enemies.clear()
    score = 0
    _spawn = 0.0


def _collide_rect(a: Object, b: Object, aw: int, ah: int, bw: int, bh: int) -> bool:
    return (
        a.position.x < b.position.x + bw
        and a.position.x + aw > b.position.x
        and a.position.y < b.position.y + bh
        and a.position.y + ah > b.position.y
    )


def update(dt: float, input_core=Input) -> bool:
    """Update game state. Returns False when game is over."""
    global _spawn, score
    if player is None:
        return False
    player.update(input_core, WIDTH)

    if input_core.was_pressed("SPACE"):
        bullets.append(Bullet.create(int(player.obj.position.x + 15), int(player.obj.position.y)))

    _spawn += dt
    if _spawn >= 2.0:
        _spawn -= 2.0
        enemies.append(Enemy.create(random.randint(0, WIDTH - 20), -20))

    for bullet in list(bullets):
        bullet.update()
        if bullet.obj.position.y < -10:
            runtime.store.remove_object(bullet.obj.id)
            bullets.remove(bullet)

    for enemy in list(enemies):
        enemy.update()
        if enemy.obj.position.y > HEIGHT:
            runtime.store.remove_object(enemy.obj.id)
            enemies.remove(enemy)

    for bullet in list(bullets):
        for enemy in list(enemies):
            if _collide_rect(bullet.obj, enemy.obj, 1, 1, 20, 20):
                runtime.store.remove_object(bullet.obj.id)
                runtime.store.remove_object(enemy.obj.id)
                bullets.remove(bullet)
                enemies.remove(enemy)
                score += 1
                break

    for enemy in enemies:
        if _collide_rect(player.obj, enemy.obj, 30, 10, 20, 20):
            return False

    return True


def draw() -> None:
    runtime.store.render(None)


__all__ = ["create", "init_game", "update", "draw", "WIDTH", "HEIGHT", "score"]
