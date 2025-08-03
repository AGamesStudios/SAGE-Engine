from sage_engine import render


def test_frame_spike_logs_warning(capsys):
    render.begin_frame()
    render.present(memoryview(b""))
    render.begin_frame()
    render._frame_start_ns -= 10_000_000  # simulate 10ms frame
    render.present(memoryview(b""))
    out = capsys.readouterr().out
    assert "Frame time spike" in out
