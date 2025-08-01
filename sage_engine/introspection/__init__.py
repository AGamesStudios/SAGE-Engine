"""Editor introspection APIs for SAGE Engine."""

from .api import IntrospectionAPI
from .. import core

api = IntrospectionAPI()
core.expose("introspection", api)
