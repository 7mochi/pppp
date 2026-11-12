"""Mod parsing and cleanup."""

from __future__ import annotations

import ctypes
from collections.abc import Generator
from contextlib import contextmanager
from typing import Any

from pppp._ctypes import _native
from pppp.errors import ModsError


def _mods_error(status: int) -> Exception:
    if status == _native.Pppp_Allocation:
        return MemoryError()
    return ModsError("invalid mod specification")


@contextmanager
def parse(
    mods: str | int | None, ruleset: int, classic: bool
) -> Generator[Any, None, None]:
    """Parse a mod string or bitmask and cleanup afterward."""
    handle = None
    try:
        if mods is not None or classic:
            native = _native.PpppNativeMods()
            status = _native.Pppp_Mods_Create(ctypes.byref(native))
            if status != _native.Pppp_Success:
                raise _mods_error(status)
            handle = native.handle

        if mods is not None:
            if isinstance(mods, bool):
                raise TypeError("mods must be a str or an int")

            if isinstance(mods, int):
                if mods < 0 or mods > 0xFFFFFFFF:
                    raise ModsError("invalid mod specification")

                status = _native.Pppp_Mods_FromLegacy(handle, mods, ruleset)
                if status != _native.Pppp_Success:
                    raise _mods_error(status)
            else:
                status = _native.Pppp_Mods_Parse(handle, mods.encode("utf-8"))
                if status != _native.Pppp_Success:
                    raise _mods_error(status)

        if classic:
            status = _native.Pppp_Mods_AddClassic(handle)
            if status != _native.Pppp_Success:
                raise _mods_error(status)

        yield handle
    finally:
        if bool(handle):
            _native.Pppp_Mods_Destroy(handle)
