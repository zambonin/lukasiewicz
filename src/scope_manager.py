"""scope_manager.py

Defines two helper functions that mimic Łukasiewicz
behaviour regarding lexical scopes. Based on [1].

[1] http://stackoverflow.com/a/21795428"""

from __future__ import absolute_import
from sys import modules

__context__ = {}


def s_context():
    """Saves the state of a namespace."""
    __context__.update(modules[__name__].__dict__)


def r_context():
    """Restores the state of a namespace."""
    for _ in list(modules[__name__].__dict__.keys()):
        if _ not in __context__:
            del modules[__name__].__dict__[_]

    modules[__name__].__dict__.update(__context__)
