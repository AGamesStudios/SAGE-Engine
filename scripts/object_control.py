import glfw
import glm

def update(role, delta_time, platform):
    role.transform.rotate(50 * delta_time, glm.vec3(0, 1, 0))
    speed = 2.0 * delta_time
    if glfw.get_key(platform.window.window, glfw.KEY_UP) == glfw.PRESS:
        role.transform.translate(glm.vec3(0, 0, -speed))
    if glfw.get_key(platform.window.window, glfw.KEY_DOWN) == glfw.PRESS:
        role.transform.translate(glm.vec3(0, 0, speed))
    if glfw.get_key(platform.window.window, glfw.KEY_LEFT) == glfw.PRESS:
        role.transform.translate(glm.vec3(-speed, 0, 0))
    if glfw.get_key(platform.window.window, glfw.KEY_RIGHT) == glfw.PRESS:
        role.transform.translate(glm.vec3(speed, 0, 0))