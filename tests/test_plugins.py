from sage_sdk import plugins


def test_register_and_load_plugin(tmp_path):
    calls = []

    def init_engine(engine):
        calls.append('engine')

    plugins.register_plugin('engine', init_engine)
    plugins.load_plugins('engine', object(), paths=[tmp_path])
    assert calls == ['engine']
