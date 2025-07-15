from sage_engine.editor_api import EDITOR_API_VERSION

def test_editor_api_version_tuple():
    assert isinstance(EDITOR_API_VERSION, tuple) and len(EDITOR_API_VERSION) == 2
