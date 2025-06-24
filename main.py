import sys
from sage_editor import main as run

if __name__ == '__main__':
    try:
        sys.exit(run(sys.argv))
    except Exception:
        import traceback
        traceback.print_exc()
        sys.exit(1)
