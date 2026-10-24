"""Parsing a mod specification into the opaque `pppp_mods` handle the calculators take."""

from __future__ import annotations

import ctypes
from collections.abc import Iterator
from contextlib import contextmanager
from typing import Any

from . import _native
from .errors import ModsError


def _mods_error(status: int) -> ModsError:
    if status == _native.PPPP_ALLOCATION:
        raise MemoryError
    return ModsError("invalid mod specification")


@contextmanager
def parse(mods: str | int | None, ruleset: int, classic: bool) -> Iterator[Any]:
    """Hold a `pppp_mods` for the duration of the `with` block, freeing it on exit.

    `mods` is a specification such as `"HD,DT"` or the stable bitmask; `None` means no mods, and
    `classic` adds CL, which the difficulty calculator only honours for osu!.
    """
    handle = ctypes.POINTER(_native.pppp_mods)()
    try:
        if mods is not None:
            if isinstance(mods, bool):
                raise TypeError("mods must be a str or an int")
            if isinstance(mods, int):
                if mods < 0 or mods > 0xFFFFFFFF:
                    raise ModsError("invalid mod specification")
                status = _native.pppp_mods_from_legacy(mods, ruleset, ctypes.byref(handle))
                if status != 0:
                    raise _mods_error(status)
            elif isinstance(mods, str):
                status = _native.pppp_mods_parse(mods.encode("utf-8"), ctypes.byref(handle))
                if status != 0:
                    raise _mods_error(status)
            else:
                raise TypeError("mods must be a str or an int")
        if classic:
            if not bool(handle):
                status = _native.pppp_mods_from_legacy(0, ruleset, ctypes.byref(handle))
                if status != 0:
                    raise _mods_error(status)
            status = _native.pppp_mods_add_classic(handle)
            if status != 0:
                raise _mods_error(status)
        yield handle
    finally:
        if bool(handle):
            _native.pppp_mods_free(handle)
