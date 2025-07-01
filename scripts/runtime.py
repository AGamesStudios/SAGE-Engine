"""Run a SAGE scene or project using the runtime engine."""
import sys
from engine.core.engine import main

if __name__ == "__main__":
    main(sys.argv[1:])
