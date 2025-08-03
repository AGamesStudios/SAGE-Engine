from sage_engine.render import stats as render_stats

def test_fps_spike_logs_warning(capsys):
    render_stats.update_fps(16.6)
    render_stats.update_fps(16.7)
    render_stats.update_fps(33.0)  # 30fps vs ~60 avg -> spike
    out = capsys.readouterr().out
    assert "Spike detected" in out
