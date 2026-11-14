"""osu! difficulty and performance calculations."""

from __future__ import annotations

from pppp import library

library.prepare(library.search_directory())

from pppp.attributes import (
    CatchDifficultyAttributes,
    CatchPerformanceAttributes,
    DifficultyAttributes,
    ManiaDifficultyAttributes,
    ManiaPerformanceAttributes,
    OsuDifficultyAttributes,
    OsuPerformanceAttributes,
    PerformanceAttributes,
    TaikoDifficultyAttributes,
    TaikoPerformanceAttributes,
    TimedDifficultyAttributes,
)
from pppp.beatmap import Beatmap, BeatmapDifficulty
from pppp.difficulty import Difficulty
from pppp.hit_result import HitResult
from pppp.performance import Performance
from pppp.ruleset import Ruleset
from pppp.strains import CatchStrains, ManiaStrains, OsuStrains, Strains, TaikoStrains
from pppp.backend import __version__, backend
from pppp.errors import Error, ModsError, ParseError

__all__ = [
    "Beatmap",
    "BeatmapDifficulty",
    "CatchDifficultyAttributes",
    "CatchPerformanceAttributes",
    "CatchStrains",
    "Difficulty",
    "DifficultyAttributes",
    "HitResult",
    "ManiaDifficultyAttributes",
    "ManiaPerformanceAttributes",
    "ManiaStrains",
    "OsuDifficultyAttributes",
    "OsuPerformanceAttributes",
    "OsuStrains",
    "Performance",
    "PerformanceAttributes",
    "Ruleset",
    "Strains",
    "TaikoDifficultyAttributes",
    "TaikoPerformanceAttributes",
    "TaikoStrains",
    "TimedDifficultyAttributes",
    "Error",
    "ModsError",
    "ParseError",
    "__version__",
    "backend",
    "library",
]
