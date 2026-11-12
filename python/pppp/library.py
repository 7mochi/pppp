"""Find and load the pppp_c shared library."""

from __future__ import annotations

import os
import sys
from pathlib import Path

from pppp._ctypes import ctypes_loader

_PLATFORM_NAMES = {
    "win32": ("pppp_c.dll",),
    "darwin": ("libpppp_c.dylib", "pppp_c.dylib"),
}
_DEFAULT_NAMES = ("libpppp_c.so", "pppp_c.so")


def _names() -> tuple[str, ...]:
    return _PLATFORM_NAMES.get(sys.platform, _DEFAULT_NAMES)


def _candidate(path: Path) -> Path | None:
    if path.is_file():
        return path

    if path.is_dir():
        for name in _names():
            candidate = path / name
            if candidate.is_file():
                return candidate

    return None


def library_path() -> Path:
    """Return the path to the pppp_c library."""
    here = Path(__file__).resolve().parent
    configured = os.environ.get("PPPP_LIBRARY")
    candidates: list[Path] = []

    if configured:
        candidates.append(Path(configured))
    for name in _names():
        candidates.append(here / name)

    root = here.parents[1]
    for mode in ("debug", "release"):
        for name in _names():
            candidates.extend(sorted(root.glob(f"build/**/{mode}/{name}"), reverse=True))

    for candidate in candidates:
        found = _candidate(candidate)
        if found is not None:
            return found

    searched = "\n".join(str(candidate) for candidate in candidates)

    raise ImportError(
        f"cannot find pppp_c, build pppp or set PPPP_LIBRARY to the shared library\nsearched:\n{searched}"
    )


def search_directory() -> Path:
    """Return the directory containing pppp_c."""
    return library_path().parent


def prepare(path: Path) -> None:
    """Register a directory for loading pppp_c."""
    if sys.platform == "win32":
        try:
            _ = os.add_dll_directory(str(path))
        except (AttributeError, OSError):
            pass
    ctypes_loader.add_library_search_dirs([str(path)])  # type: ignore[no-untyped-call]
