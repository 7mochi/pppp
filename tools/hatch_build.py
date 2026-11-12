"""
Hook for building wheels with the hatchling build backend.

- Set wheel to being platform-specific (not pure python).
- The native library is compiled beforehand and included in the wheel.
- Support cross-platform wheel building with a custom env var.
- Note that for sdist we go into pure-python mode.
"""

from __future__ import annotations

import os
import sysconfig
from pathlib import Path

from hatchling.builders.hooks.plugin.interface import BuildHookInterface


class CustomBuildHook(BuildHookInterface):
    def initialize(self, version: str, build_data: dict[str, object]) -> None:
        # See https://hatch.pypa.io/latest/plugins/builder/wheel/#build-data

        # We only tag wheels with a platform, sdist stays pure-python.
        if self.target_name != "wheel":
            return

        root = Path(self.root)
        library = _find(root, "pppp_c")
        engine = _find(root, "fosu_engine")
        if library is None or engine is None:
            raise RuntimeError(
                "pppp_c and the parser engine are required; run `make release` first"
            )

        build_data["pure_python"] = False

        wheel_tag = os.getenv("PPPP_WHEEL_TAG")
        if wheel_tag:
            # A cross-platform build
            build_data["tag"] = f"py3-none-{wheel_tag}"
        else:
            # ctypes doesn't depend on the CPython ABI. A cpXXX tag would exclude interpreters
            # that can load the wheel.
            platform = sysconfig.get_platform().replace("-", "_").replace(".", "_")
            build_data["tag"] = f"py3-none-{platform}"

        # The loader looks for both libraries beside __init__.py. xmake leaves them under build/.
        force_include = build_data.get("force_include")
        if not isinstance(force_include, dict):
            force_include = {}
            build_data["force_include"] = force_include

        force_include[str(library)] = f"pppp/{library.name}"
        force_include[str(engine)] = f"pppp/{engine.name}"


def _find(root: Path, needle: str) -> Path | None:
    # Release builds are what ships. Debug is only a local convenience.
    for mode in ("release", "debug"):
        for path in sorted(root.glob(f"build/**/{mode}/*")):
            if path.is_file() and needle in path.name and path.suffix in (".so", ".dylib", ".dll"):
                return path
    return None
