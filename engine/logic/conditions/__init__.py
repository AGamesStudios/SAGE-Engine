"""Built-in conditions for the SAGE engine.

Each condition lives in its own module to keep the system modular. Importing
this package registers all built-in conditions via their decorators.
"""

from ..base import Condition, register_condition, resolve_value  # re-export

# import submodules so decorators run
from . import onstart
from . import keypressed
from . import keyreleased
from . import inputstate
from . import objectvisible
from . import variablecompare
from . import objectvariablecompare

__all__ = [
    'Condition', 'register_condition', 'resolve_value',
    'OnStart', 'KeyPressed', 'KeyReleased', 'InputState',
    'ObjectVisible', 'VariableCompare', 'ObjectVariableCompare',
]

from .onstart import OnStart
from .keypressed import KeyPressed
from .keyreleased import KeyReleased
from .inputstate import InputState
from .objectvisible import ObjectVisible
from .variablecompare import VariableCompare
from .objectvariablecompare import ObjectVariableCompare
