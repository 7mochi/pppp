"""The helpers over the generated declarations that every other module uses."""

from __future__ import annotations

import ctypes
from typing import cast

from . import _native
from .errors import Error


def message(status: int) -> str:
    """The native description of a `pppp_result`."""
    value = _native.pppp_result_message(status)
    if value is None:
        return f"pppp error {status}"
    return cast(str, value.decode("utf-8", "replace"))


def check(status: int, error: type[Error] = Error) -> None:
    """Raise unless `status` is `PPPP_OK`, turning an allocation failure into `MemoryError`."""
    if status == 0:
        return
    if status == _native.PPPP_ALLOCATION:
        raise MemoryError
    raise error(message(status))


def difficulty_options(
    mods: ctypes.c_void_p, ruleset: int | None, clock_rate: float | None
) -> _native.pppp_difficulty_options:
    """Build a `pppp_difficulty_options` with only the fields the caller set."""
    from .library import PPPP_DIFFICULTY_OPTIONS_VERSION

    options = _native.pppp_difficulty_options()
    options.version = PPPP_DIFFICULTY_OPTIONS_VERSION
    options.mods = mods
    if ruleset is not None:
        options.ruleset = int(ruleset)
        options.has_ruleset = 1
    if clock_rate is not None:
        options.clock_rate = float(clock_rate)
        options.has_clock_rate = 1
    return options
