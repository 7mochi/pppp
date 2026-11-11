"""The values of the native library the package exposes as-is."""

from __future__ import annotations

from . import _native
from .library import PPPP_C_ABI_VERSION

if _native.pppp_abi_version() != PPPP_C_ABI_VERSION:
    raise ImportError("pppp_c has an incompatible C ABI")

__version__ = _native.pppp_version().decode("utf-8")
backend = _native.pppp_backend().decode("utf-8")
