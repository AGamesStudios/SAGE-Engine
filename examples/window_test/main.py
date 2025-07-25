from sage_engine.window import poll, present, should_close
from sage_engine import core_boot, core_reset


def main() -> None:
    core_boot()
    try:
        while not should_close():
            poll()
            present()
    finally:
        core_reset()


if __name__ == "__main__":
    main()
