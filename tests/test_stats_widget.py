import types
import time

from tests.test_viewport_sync import _setup_qt


def test_stats_widget_tracks_fps(monkeypatch):
    _setup_qt(monkeypatch)
    times = iter([0.0, 0.5, 1.2])
    monkeypatch.setattr(time, "perf_counter", lambda: next(times))
    from sage_editor.plugins.editor_widgets import StatsWidget

    window = types.SimpleNamespace(scene=types.SimpleNamespace(objects=[1]), _engine=None)
    w = StatsWidget(window)
    w.update_stats()
    w.update_stats()
    assert "FPS" in getattr(w.label, "text", "")
