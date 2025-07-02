"""Built-in logic actions for the SAGE engine.

Each action lives in its own module for modularity. Importing this package
registers all built-ins via their decorators so they are available through
``engine.logic``.
"""

from ..base import Action, register_action, resolve_value  # re-export
from ...utils.log import logger

# pull in built-in actions
from . import print as _print  # noqa: F401
from . import movedirection  # noqa: F401
from . import setvariable  # noqa: F401
from . import modifyvariable  # noqa: F401
from . import setobjectvariable  # noqa: F401
from . import modifyobjectvariable  # noqa: F401
from . import showobject  # noqa: F401
from . import hideobject  # noqa: F401
from . import setposition  # noqa: F401
from . import setrotation  # noqa: F401
from . import setscale  # noqa: F401
from . import flip  # noqa: F401
from . import setalpha  # noqa: F401
from . import setcolor  # noqa: F401
from . import setsprite  # noqa: F401
from . import setzorder  # noqa: F401
from . import renameobject  # noqa: F401
from . import deleteobject  # noqa: F401
from . import createobject  # noqa: F401
from . import setcamerazoom  # noqa: F401
from . import setcamerasize  # noqa: F401
from . import setactivecamera  # noqa: F401
from . import setkeepaspect  # noqa: F401
from . import playsound  # noqa: F401

__all__ = [
    'Action', 'register_action', 'resolve_value', 'logger',
    'Print', 'MoveDirection', 'SetVariable', 'ModifyVariable',
    'SetObjectVariable', 'ModifyObjectVariable', 'ShowObject', 'HideObject',
    'SetPosition', 'SetRotation', 'SetScale', 'Flip', 'SetAlpha', 'SetColor',
    'SetSprite', 'SetZOrder', 'RenameObject', 'DeleteObject', 'CreateObject',
    'SetCameraZoom', 'SetCameraSize', 'SetActiveCamera', 'SetKeepAspect',
    'PlaySound',
]

# expose action classes at the package level
from .print import Print
from .movedirection import MoveDirection
from .setvariable import SetVariable
from .modifyvariable import ModifyVariable
from .setobjectvariable import SetObjectVariable
from .modifyobjectvariable import ModifyObjectVariable
from .showobject import ShowObject
from .hideobject import HideObject
from .setposition import SetPosition
from .setrotation import SetRotation
from .setscale import SetScale
from .flip import Flip
from .setalpha import SetAlpha
from .setcolor import SetColor
from .setsprite import SetSprite
from .setzorder import SetZOrder
from .renameobject import RenameObject
from .deleteobject import DeleteObject
from .createobject import CreateObject
from .setcamerazoom import SetCameraZoom
from .setcamerasize import SetCameraSize
from .setactivecamera import SetActiveCamera
from .setkeepaspect import SetKeepAspect
from .playsound import PlaySound
