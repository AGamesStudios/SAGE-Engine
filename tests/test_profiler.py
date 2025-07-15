def test_profiler_creates_file(tmp_path):
    from sage_engine.utils.profiler import Profiler

    prof_path = tmp_path / "out.prof"
    with Profiler(str(prof_path)):
        sum(range(10))
    assert prof_path.exists()
