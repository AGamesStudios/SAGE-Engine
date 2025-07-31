import os, sys, pytest
os.environ.setdefault("PYTEST_DISABLE_PLUGIN_AUTOLOAD", "1")
os.environ.setdefault("PYTHONNOUSERSITE", "1")
sys.exit(pytest.main(["-q", "tests"]))
