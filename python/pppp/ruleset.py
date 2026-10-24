"""The ruleset ids an attribute set belongs to."""

from __future__ import annotations

from enum import IntEnum


class Ruleset(IntEnum):
    """Which ruleset an attribute set belongs to. The values are the beatmap's `Mode`."""

    OSU = 0
    TAIKO = 1
    CATCH = 2
    MANIA = 3
