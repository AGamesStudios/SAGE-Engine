from sage_engine import input, time
import pygame
from sage.events import on


def test_input_events_and_state():
    input.boot()
    events = []
    on("key_down", lambda d: events.append(("down", d["key"])))
    on("key_up", lambda d: events.append(("up", d["key"])))
    input.press_key("A")
    assert input.is_key_down("A")
    input.release_key("A")
    assert not input.is_key_down("A")
    assert ("down", "A") in events and ("up", "A") in events


def test_handle_pygame_event():
    input.boot()
    events = []
    on("key_down", lambda d: events.append(d["key"]))
    ev = pygame.event.Event(pygame.KEYDOWN, {"key": pygame.K_w})
    input.handle_pygame_event(ev)
    assert "w" in events
    assert input.is_key_down("w")


def test_time_tracking():
    time.boot()
    time.mark()
    start = time.get_time()
    dt1 = time.get_delta()
    assert dt1 >= 0
    time.wait(10)
    dt2 = time.get_delta()
    assert dt2 > 0
    assert time.get_time() >= start
