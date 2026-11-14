"""Version and parser backend of the loaded library."""

from __future__ import annotations

from pppp._ctypes import _native

__version__ = _native.Pppp_Version().decode("utf-8")
backend = _native.Pppp_Backend().decode("utf-8")
