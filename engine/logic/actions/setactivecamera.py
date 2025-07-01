from ..base import Action, register_action


@register_action('SetActiveCamera', [('camera', 'object')])
class SetActiveCamera(Action):
    """Make the given camera active."""

    def __init__(self, camera):
        self.camera = camera

    def execute(self, engine, scene, dt):
        if scene:
            scene.set_active_camera(self.camera)


