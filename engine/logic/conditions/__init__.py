"""Built-in conditions for the SAGE engine.

Each condition lives in its own module to keep the system modular. Importing
this package registers all built-in conditions via their decorators.
"""

from ..base import Condition, register_condition, resolve_value  # re-export

# import submodules so decorators run
from . import onstart  # noqa: F401
from . import keypressed  # noqa: F401
from . import keyreleased  # noqa: F401
from . import inputstate  # noqa: F401
from . import objectvisible  # noqa: F401
from . import variablecompare  # noqa: F401
from . import objectvariablecompare  # noqa: F401
from . import evalexpr  # noqa: F401

__all__ = [
    'Condition', 'register_condition', 'resolve_value',
    'OnStart', 'KeyPressed', 'KeyReleased', 'InputState',
    'ObjectVisible', 'VariableCompare', 'ObjectVariableCompare',
    'EvalExpr',
]

from .onstart import OnStart
from .keypressed import KeyPressed
from .keyreleased import KeyReleased
from .inputstate import InputState
from .objectvisible import ObjectVisible
from .variablecompare import VariableCompare
from .objectvariablecompare import ObjectVariableCompare
from .evalexpr import EvalExpr
