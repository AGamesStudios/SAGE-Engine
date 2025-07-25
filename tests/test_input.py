from sage_engine import input
import pygame
from sage.events import on


def test_key_press_and_release():
    input.boot()
    input.update()
    events = []
    on("key_down", lambda d: events.append(d["key"]))
    input.press_key("A")
    assert input.input_key_pressed("A")
    input.update()
    assert input.input_key_down("A")
    input.release_key("A")
    assert input.input_key_released("A")
    input.update()
    assert not input.input_key_down("A")
    assert "A" in events


def test_mouse_button_and_move():
    input.boot()
    input.update()
    input.move_mouse(0, 0)
    input.press_mouse("1")
    assert input.input_mouse_button_pressed("1")
    input.update()
    input.move_mouse(10, 5)
    assert input.input_mouse_delta() == (10, 5)
    input.update()
    input.release_mouse("1")
    assert input.input_mouse_button_released("1")
    input.update()


def test_handle_pygame_event_bridge():
    input.boot()
    ev = pygame.event.Event(pygame.KEYDOWN, {"key": pygame.K_SPACE})
    input.handle_pygame_event(ev)
    assert input.input_key_down("space")
