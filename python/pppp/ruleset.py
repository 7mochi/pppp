"""Ruleset ids."""

from __future__ import annotations

from enum import IntEnum


class Ruleset(IntEnum):
    """A ruleset id, matching the beatmap's Mode."""

    OSU = 0
    TAIKO = 1
    CATCH = 2
    MANIA = 3
