#!/usr/bin/env python3
"""Refresh codegen/mods/mods.json from the in-repo C# reference.

`mods.json` is the enriched upstream mod table: the same mods each ruleset exposes, with the
settings upstream's own `mods.json` lacks — each setting's default, and its `BindableNumber`
range and precision — plus the osu!stable bits per ruleset. It is committed; this script is the
only step that needs `dotnet`, and `gen_mods.py` writes the C++ from it.
"""

from __future__ import annotations

import os
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
PROJECT = HERE.parent.parent
REFERENCE = PROJECT / "reference"
REFERENCE_CSPROJ = REFERENCE / "reference.csproj"
REFERENCE_DLL = REFERENCE / "bin" / "Release" / "net8.0" / "reference.dll"
OUT = HERE / "mods.json"


def main():
    subprocess.run(["dotnet", "build", str(REFERENCE_CSPROJ), "-c", "Release", "-v", "quiet"],
                   check=True)
    environment = dict(os.environ, DOTNET_ROLL_FORWARD="Major")
    out = subprocess.run(["dotnet", str(REFERENCE_DLL), "--mods"],
                         stdout=subprocess.PIPE, env=environment, check=True).stdout
    OUT.write_bytes(out)
    print("wrote %s (%d bytes)" % (OUT, len(out)))


if __name__ == "__main__":
    main()
