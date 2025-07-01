"""Built-in logic actions for the SAGE engine.

Each action lives in its own module for modularity. Importing this package
registers all built-ins via their decorators so they are available through
``engine.logic``.
"""

from ..base import Action, register_action, resolve_value  # re-export
from ...utils.log import logger

# pull in built-in actions
from . import print as _print
from . import movedirection
from . import setvariable
from . import modifyvariable
from . import setobjectvariable
from . import modifyobjectvariable
from . import showobject
from . import hideobject
from . import setposition
from . import setrotation
from . import setscale
from . import flip
from . import setalpha
from . import setcolor
from . import setsprite
from . import setzorder
from . import renameobject
from . import deleteobject
from . import createobject
from . import setcamerazoom
from . import setcamerasize
from . import setactivecamera
from . import setkeepaspect

__all__ = [
    'Action', 'register_action', 'resolve_value', 'logger',
    'Print', 'MoveDirection', 'SetVariable', 'ModifyVariable',
    'SetObjectVariable', 'ModifyObjectVariable', 'ShowObject', 'HideObject',
    'SetPosition', 'SetRotation', 'SetScale', 'Flip', 'SetAlpha', 'SetColor',
    'SetSprite', 'SetZOrder', 'RenameObject', 'DeleteObject', 'CreateObject',
    'SetCameraZoom', 'SetCameraSize', 'SetActiveCamera', 'SetKeepAspect',
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
