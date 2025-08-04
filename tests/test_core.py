from sage_engine.core import register, boot_engine, stop


def test_phase_order():
    events: list[str] = []

    def boot(cfg: dict | None = None) -> None:
        events.append("boot")

    def update() -> None:
        events.append("update")
        stop()

    def draw() -> None:
        events.append("draw")

    def flush() -> None:
        events.append("flush")

    def shutdown() -> None:
        events.append("shutdown")

    register("boot", boot)
    register("update", update)
    register("draw", draw)
    register("flush", flush)
    register("shutdown", shutdown)

    boot_engine()

    assert events == ["boot", "update", "draw", "flush", "shutdown"]
