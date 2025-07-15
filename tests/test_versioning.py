import pytest

import sage_engine as engine


def test_require_version_pass():
    engine.require_version('0.0.1-alpha')


def test_require_version_fail():
    with pytest.raises(RuntimeError):
        engine.require_version('1.0')
