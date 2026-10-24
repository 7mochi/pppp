"""The values of the native library the package exposes as-is."""

from __future__ import annotations

from . import _native

__version__ = _native.pppp_version().decode("utf-8")
backend = _native.pppp_backend().decode("utf-8")
