"""Simplified render pipeline with separated phases."""

prepared_data = []


def prepare(scene) -> None:
    prepared_data.clear()
    for obj_id in scene.view.with_sprite():
        prepared_data.append(obj_id)


def sort() -> None:
    prepared_data.sort()


def flush() -> None:
    prepared_data.clear()
