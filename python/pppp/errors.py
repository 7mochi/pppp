"""Exceptions raised by pppp."""


class Error(Exception):
    """Base class for pppp exceptions."""


class ModsError(Error, ValueError):
    """A mods string or bitmask is invalid."""


class ParseError(Error, ValueError):
    """A beatmap couldn't be parsed."""
