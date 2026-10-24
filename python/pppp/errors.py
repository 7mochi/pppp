"""The error types raised by the package."""


class Error(Exception):
    """Base class for errors raised by pppp."""


class ModsError(Error, ValueError):
    """The mod specification or the stable mod bitmask is invalid."""


class ParseError(Error, ValueError):
    """The beatmap could not be parsed."""
