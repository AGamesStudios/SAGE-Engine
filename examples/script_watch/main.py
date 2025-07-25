from sage import emit
from sage_engine import core_boot, core_reset, framesync, time
from sage_engine.scripts_watcher import ScriptsWatcher
from sage_engine.render import render_scene
from sage_engine.object import get_objects
from sage_engine.window import poll, present


def main() -> None:
    core_boot()
    watcher = ScriptsWatcher(folder="data/scripts")
    watcher.start(0.5)
    emit("ready")
    time.mark()
    try:
        for _ in range(10):
            poll()
            watcher.scan()
            dt = time.get_delta()
            emit("update", dt)
            render_scene(get_objects())
            present()
            framesync.regulate()
            time.mark()
    finally:
        watcher.stop()
        core_reset()


if __name__ == "__main__":
    main()
