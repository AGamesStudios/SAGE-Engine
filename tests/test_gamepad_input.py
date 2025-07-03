import sys
import types


def _stub_sdl(monkeypatch):
    mod = types.SimpleNamespace()
    mod.SDL_INIT_GAMECONTROLLER = 0
    mod.SDL_NumJoysticks = lambda: 1
    mod.SDL_IsGameController = lambda i: True
    mod.SDL_GameControllerOpen = lambda i: object()
    mod.SDL_GameControllerUpdate = lambda: None
    mod.SDL_GameControllerGetButton = lambda c, b: 1
    mod.SDL_GameControllerClose = lambda c: None
    mod.SDL_Quit = lambda: None
    mod.SDL_Init = lambda f: 0
    monkeypatch.setitem(sys.modules, "sdl2", mod)


def test_gamepad_backend(monkeypatch):
    _stub_sdl(monkeypatch)
    from engine.inputs.gamepad import GamepadInput
    gp = GamepadInput()
    gp.poll()
    assert gp.is_button_down(0)
    gp.shutdown()
