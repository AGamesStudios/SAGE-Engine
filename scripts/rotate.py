def update(role, delta_time):
    """Пример скрипта: вращение объекта вокруг оси Y"""
    role.transform.rotation.y += 50 * delta_time
    role.transform.rotation.x += 50 * delta_time