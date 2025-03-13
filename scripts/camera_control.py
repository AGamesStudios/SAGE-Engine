import glfw
import glm

def update(camera, delta_time, platform):
    sensitivity = 0.1
    speed = 5.0 * delta_time

    # Получаем положение курсора и сбрасываем его в центр окна
    cursor_x, cursor_y = glfw.get_cursor_pos(platform.window.window)
    glfw.set_cursor_pos(platform.window.window, platform.window.width / 2, platform.window.height / 2)
    delta_x = cursor_x - platform.window.width / 2
    delta_y = cursor_y - platform.window.height / 2

    # Обновляем углы поворота камеры:
    # - При движении мыши вправо увеличивается yaw, при движении вверх уменьшается pitch.
    camera.transform.rotation.y += delta_x * sensitivity   # yaw
    camera.transform.rotation.x += -delta_y * sensitivity  # pitch
    camera.transform.rotation.x = glm.clamp(camera.transform.rotation.x, -89.0, 89.0)

    # Для корректного вычисления направления камеры смещаем yaw на -90 градусов.
    # Это нужно, чтобы при camera.transform.rotation.y == 0 камера смотрела вдоль -Z.
    yaw   = glm.radians(camera.transform.rotation.y - 90.0)
    pitch = glm.radians(camera.transform.rotation.x)

    # Вычисляем вектор направления "вперёд" с учетом поворота камеры
    forward = glm.vec3(
        glm.cos(yaw) * glm.cos(pitch),
        glm.sin(pitch),
        glm.sin(yaw) * glm.cos(pitch)
    )
    forward = glm.normalize(forward)

    # Вычисляем векторы "право" и "вверх" для локальной системы координат камеры
    right = glm.normalize(glm.cross(forward, glm.vec3(0, 1, 0)))
    up    = glm.normalize(glm.cross(right, forward))

    # Движение камеры относительно её локальной системы координат:
    # W - вперёд (в направлении взгляда камеры),
    # S - назад, A - влево, D - вправо, SPACE - вверх, LEFT_SHIFT - вниз.
    if glfw.get_key(platform.window.window, glfw.KEY_W) == glfw.PRESS:
        camera.transform.translate(speed * forward)
    if glfw.get_key(platform.window.window, glfw.KEY_S) == glfw.PRESS:
        camera.transform.translate(-speed * forward)
    if glfw.get_key(platform.window.window, glfw.KEY_A) == glfw.PRESS:
        camera.transform.translate(-speed * right)
    if glfw.get_key(platform.window.window, glfw.KEY_D) == glfw.PRESS:
        camera.transform.translate(speed * right)
    if glfw.get_key(platform.window.window, glfw.KEY_SPACE) == glfw.PRESS:
        camera.transform.translate(speed * up)
    if glfw.get_key(platform.window.window, glfw.KEY_LEFT_SHIFT) == glfw.PRESS:
        camera.transform.translate(-speed * up)

    if glfw.get_key(platform.window.window, glfw.KEY_C) == glfw.PRESS:
        platform.set_active_camera((platform.active_camera_idx + 1) % len(platform.cameras))
