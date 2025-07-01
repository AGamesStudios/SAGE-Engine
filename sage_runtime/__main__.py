from engine.core.engine import main as _main
import sys

def main(argv=None):
    _main(argv)

if __name__ == "__main__":
    main(sys.argv[1:])
