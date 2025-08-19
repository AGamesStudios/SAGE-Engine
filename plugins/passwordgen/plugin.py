
# SAGE Engine plugin: passwordgen
from .generator import generate

def register(api):
    # provide service factory
    def factory():
        return lambda **kw: generate(**kw)
    api.provide_service('password_generator', factory)
    # command
    def cmd_generate(length=16, sets=('upper','lower','digits','symbols'),
                     ensure_each=True, avoid_ambiguous=True, custom=None):
        return generate(length=length, sets=sets, ensure_each=ensure_each, avoid_ambiguous=avoid_ambiguous, custom=custom)
    api.register_command('password.generate', cmd_generate)
    # simple event
    api.events.on('engine.start', lambda **k: None)
