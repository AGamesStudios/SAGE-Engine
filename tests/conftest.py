import importlib.util
import pytest


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption("--skip-audio", action="store_true", help="skip audio tests")
    parser.addoption("--skip-physics", action="store_true", help="skip physics tests")


def pytest_configure(config: pytest.Config) -> None:
    config.addinivalue_line("markers", "audio: mark test requiring audio modules")
    config.addinivalue_line("markers", "physics: mark test requiring physics modules")


def _has_module(name: str) -> bool:
    return importlib.util.find_spec(name) is not None


def pytest_collection_modifyitems(config: pytest.Config, items: list[pytest.Item]) -> None:
    skip_audio = pytest.mark.skip(reason="audio dependencies not available or skipped")
    skip_physics = pytest.mark.skip(reason="physics dependencies not available or skipped")
    for item in items:
        if 'audio' in item.keywords:
            if config.getoption("--skip-audio") or not _has_module('simpleaudio'):
                item.add_marker(skip_audio)
        if 'physics' in item.keywords:
            if config.getoption("--skip-physics") or not _has_module('Box2D'):
                item.add_marker(skip_physics)
