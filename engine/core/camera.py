from dataclasses import dataclass, field
from .objects import register_object
from ..logic import EventSystem, Event, condition_from_dict, action_from_dict
from ..log import logger
from .. import units

@register_object(
    'camera',
    [
        ('x', None),
        ('y', None),
        ('z', None),
        ('width', None),
        ('height', None),
        ('zoom', None),
        ('name', None),
        ('active', 'active'),
        ('metadata', 'metadata'),
    ],
)
@dataclass(slots=True)
class Camera:
    """2D camera object that can be placed in a scene."""

    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    width: int = 640
    height: int = 480
    zoom: float = 1.0
    name: str = "Camera"
    active: bool = False
    type: str = "camera"
    metadata: dict = field(default_factory=dict)
    events: list = field(default_factory=list)
    event_system: EventSystem | None = field(init=False, default=None)

    def view_rect(self) -> tuple[float, float, float, float]:
        """Return the visible world rectangle with the camera centered."""
        scale = units.UNITS_PER_METER
        w = (self.width / self.zoom) * scale
        h = (self.height / self.zoom) * scale
        left = self.x * scale - w / 2
        bottom = self.y * scale - h / 2
        return (left, bottom, w, h)

    def update(self, dt: float) -> None:
        """Camera objects currently have no behaviour."""
        pass

    def draw(self, surface) -> None:
        """Camera objects are not drawn with sprites."""
        pass

    def build_event_system(self, objects, variables) -> EventSystem:
        """Build and store an EventSystem from the attached events."""
        es = EventSystem(variables=variables)
        for evt in getattr(self, "events", []):
            if not isinstance(evt, dict):
                continue
            conditions = []
            for cond in evt.get("conditions", []):
                if not isinstance(cond, dict):
                    continue
                cobj = condition_from_dict(cond, objects, variables)
                if cobj is not None:
                    conditions.append(cobj)
                else:
                    logger.warning('Skipped invalid condition %s', cond)
            actions = []
            for act in evt.get("actions", []):
                if not isinstance(act, dict):
                    continue
                aobj = action_from_dict(act, objects)
                if aobj is not None:
                    actions.append(aobj)
                else:
                    logger.warning('Skipped invalid action %s', act)
            es.add_event(Event(conditions, actions, evt.get("once", False)))
        self.event_system = es
        return es
