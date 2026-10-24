"""Locating and loading `pppp_c`, and the ABI version check."""

from __future__ import annotations

import ctypes
import os
import sys
from pathlib import Path

from . import ctypes_loader

_LIBRARY_NAMES = {"win32": "pppp_c.dll", "darwin": "pppp_c.dylib"}

# The constants the header declares as macros. ctypesgen does not read macros, so they are written
# here; `include/pppp/capi.h` is the source of truth for the values.
PPPP_C_ABI_VERSION = 1
PPPP_HIT_RESULT_COUNT = 18
PPPP_DIFFICULTY_OPTIONS_VERSION = 1
PPPP_PERFORMANCE_OPTIONS_VERSION = 1


def library_name() -> str:
    """The file name of `pppp_c` on this platform."""
    return _LIBRARY_NAMES.get(sys.platform, "pppp_c.so")


def library_path() -> Path:
    """The path of `pppp_c`, honouring `PPPP_LIBRARY`, the package directory and `build/`."""
    name = library_name()
    here = Path(__file__).resolve().parent
    candidates = []
    configured = os.environ.get("PPPP_LIBRARY")
    if configured:
        candidates.append(Path(configured))
    candidates.append(here / name)
    candidates.append(here.parents[1] / "build" / name)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise ImportError(
        f"cannot find {name}; build pppp or set PPPP_LIBRARY to the shared library"
    )


def search_directory() -> Path:
    """The directory the generated loader has to scan before `_native` is imported."""
    return library_path().parent


def prepare(path: Path) -> None:
    """Let the platform find a library beside `path`."""
    if sys.platform == "win32":
        try:
            os.add_dll_directory(str(path))
        except (AttributeError, OSError):
            pass
    ctypes_loader.add_library_search_dirs([str(path)])  # type: ignore[no-untyped-call]
