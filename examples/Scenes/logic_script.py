def register(scene):
    scene.events.append({
        "name": "ScriptEvent",
        "conditions": [{"type": "OnStart"}],
        "actions": [{"type": "Print", "text": "Script logic executed"}]
    })
