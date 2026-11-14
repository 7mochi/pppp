"""Helpers for the generated ctypes bindings."""

from __future__ import annotations

import ctypes
from collections.abc import Callable
from typing import Any, cast

from pppp._ctypes import _native
from pppp.errors import Error


def _message(status: int) -> str:
    value = _native.Pppp_ErrorHandler_GetLastMessage()
    if value is None:
        return f"pppp error {status}"
    return cast(bytes, value).decode("utf-8", "replace")


def check_status(status: int, error: type[Error] = Error) -> None:
    if status == _native.Pppp_Success:
        return
    if status == _native.Pppp_Allocation:
        raise MemoryError
    raise error(_message(status))


def read_string(getter: Callable[..., int], handle: Any) -> str:
    size = _native.PpppInt32(0)
    status = getter(handle, None, ctypes.byref(size))
    if status != _native.Pppp_BufferSizeQuery:
        check_status(status)
    if size.value <= 0:
        return ""
    buffer = (ctypes.c_char * size.value)()
    status = getter(handle, buffer, ctypes.byref(size))
    check_status(status)
    return buffer.raw[: max(size.value - 1, 0)].decode("utf-8", "replace")


def read_doubles(getter: Callable[..., int], handle: Any) -> list[float]:
    size = _native.PpppInt32(0)
    status = getter(handle, None, ctypes.byref(size))
    if status != _native.Pppp_BufferSizeQuery:
        check_status(status)
    if size.value <= 0:
        return []
    buffer = (ctypes.c_double * size.value)()
    status = getter(handle, buffer, ctypes.byref(size))
    check_status(status)
    return [float(buffer[index]) for index in range(size.value)]
