# Demo Scenes (ECS)

## ECSBasicDemo (player + enemies + pickup)
- Файл: `Examples/ECSBasicDemo.cpp`
- Состав:
  - Игрок: Transform, Sprite, PlayerTag, Velocity, PlayerMovement, Health/Stats, Collider, CameraFollow.
  - Враги: Transform, Sprite, EnemyTag, Velocity, Collider, простое патрулирование.
  - Предмет: Transform, Sprite (transparent), Collider — лечит игрока при пересечении.
  - HudRenderSystem показывает HP/EN и [PAUSED].
- Порядок систем: InputState → PlayerInput → Movement → Collision → Stats → Animation → ParticleSystem → Audio → CameraFollow → SpriteRender → HudRender.

## Как подключить новые сцены
1. Создайте класс от `ECSGame`.
2. В `OnECSCreate` создайте сущности с нужными компонентами (Transform/Sprite/Collider/Health и т.д.).
3. Логику сцены размещайте в `OnECSUpdate` (урон при коллизии, патруль, обработка предметов).
4. Передайте цель камеры через `SetCameraTarget(entity)`.

## Коллизии
- Используйте `ColliderComponent` (AABB) + `CollisionSystem`. Поле `colliding` выставляется автоматически; реакцию реализуйте в логике сцены.

## Частицы
- Вешайте `ParticleEmitterComponent` на сущность: задайте `config` и `emissionRate`. Эффекты (дым/взрыв) будут обновлены и отрисованы в `ParticleSystemSystem`.

## Аудио
- Через `AudioComponent` задавайте `sound`, `loop`, `playRequested`, `volume`. Система аудио обновляется после логики и частиц.

## HUD
- HudRenderSystem выводит HP/EN первого `PlayerTag` и флаг паузы. Расширяйте сценой при необходимости (очки, таймеры).
