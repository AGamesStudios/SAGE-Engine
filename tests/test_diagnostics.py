from engine.utils.diagnostics import analyze_exception


def test_analyze_exception_basic():
    try:
        1/0
    except ZeroDivisionError as exc:
        tb = exc.__traceback__
        summary = analyze_exception(ZeroDivisionError, exc, tb)
    assert "ZeroDivisionError" in summary
    assert "division by zero" in summary

