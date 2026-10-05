"""pppp: osu! difficulty and performance points, in native code."""

from __future__ import annotations

import os
from collections.abc import Mapping
from dataclasses import dataclass

from . import _core
from ._core import Beatmap as Beatmap
from ._core import Error as Error
from ._core import ModsError as ModsError
from ._core import ParseError as ParseError
from ._model import (
    BeatmapDifficulty,
    BreakPeriod,
    CatchDifficultyAttributes,
    CatchPerformanceAttributes,
    CatchStrains,
    DifficultyAttributes,
    HitObject,
    HitResult,
    ManiaDifficultyAttributes,
    ManiaPerformanceAttributes,
    ManiaStrains,
    OsuDifficultyAttributes,
    OsuPerformanceAttributes,
    OsuStrains,
    PerformanceAttributes,
    Ruleset,
    Slider,
    SliderEvent,
    Strains,
    TaikoDifficultyAttributes,
    TaikoPerformanceAttributes,
    TaikoStrains,
    TimedDifficultyAttributes,
    TimingPoint,
    Vector2,
)

__version__: str = _core.version


def _from_file(cls: type[Beatmap], path: str | bytes | os.PathLike[str] | os.PathLike[bytes]) -> Beatmap:
    """Parse a `Beatmap` by providing a path to a `.osu` file."""
    with open(path, "rb") as file:
        return cls.from_bytes(file.read())


_from_file.__name__ = "from_file"
_from_file.__qualname__ = "Beatmap.from_file"
Beatmap.from_file = classmethod(_from_file)  # type: ignore[assignment,method-assign]


@dataclass(frozen=True, kw_only=True)
class Difficulty:
    """Difficulty calculator on maps of any mode."""

    mods: str | int = ""
    ruleset: Ruleset | None = None
    clock_rate: float | None = None

    def calculate(self, beatmap: Beatmap) -> DifficultyAttributes:
        """Perform the difficulty calculation."""
        return _core.calculate_difficulty(
            beatmap,
            self.mods,
            None if self.ruleset is None else int(self.ruleset),
            self.clock_rate,
        )

    def calculate_timed(self, beatmap: Beatmap) -> list[TimedDifficultyAttributes]:
        """Calculates the difficulty of the beatmap using a specific mod combination and returns a set of
        TimedDifficultyAttributes representing the difficulty at every relevant time value in the
        beatmap."""
        return _core.calculate_timed_difficulty(
            beatmap,
            self.mods,
            None if self.ruleset is None else int(self.ruleset),
            self.clock_rate,
        )

    def strains(self, beatmap: Beatmap) -> Strains:
        """Perform the difficulty calculation but instead of evaluating the skill
        strains, return them as is.

        Suitable to plot the difficulty of a map over time."""
        return _core.calculate_strains(
            beatmap,
            self.mods,
            None if self.ruleset is None else int(self.ruleset),
            self.clock_rate,
        )


@dataclass(frozen=True, kw_only=True)
class Performance:
    """Performance calculator on maps of any mode."""

    mods: str | int = ""
    max_combo: int | None = None
    accuracy: float | None = None
    misses: int | None = None
    statistics: Mapping[HitResult, int] | None = None
    legacy_total_score: int | None = None

    def calculate(
        self, beatmap: Beatmap, attributes: DifficultyAttributes | None = None
    ) -> PerformanceAttributes:
        """Perform the performance calculation for the map's or the attributes' mode."""
        return _core.calculate_performance(
            beatmap,
            self.mods,
            self.max_combo,
            self.accuracy,
            self.misses,
            self.statistics,
            self.legacy_total_score,
            attributes,
        )


__all__ = [
    "Beatmap",
    "BeatmapDifficulty",
    "BreakPeriod",
    "CatchDifficultyAttributes",
    "CatchPerformanceAttributes",
    "CatchStrains",
    "Difficulty",
    "DifficultyAttributes",
    "Error",
    "HitObject",
    "HitResult",
    "ManiaDifficultyAttributes",
    "ManiaPerformanceAttributes",
    "ManiaStrains",
    "ModsError",
    "OsuDifficultyAttributes",
    "OsuPerformanceAttributes",
    "OsuStrains",
    "ParseError",
    "Performance",
    "PerformanceAttributes",
    "Ruleset",
    "Slider",
    "SliderEvent",
    "Strains",
    "TaikoDifficultyAttributes",
    "TaikoPerformanceAttributes",
    "TaikoStrains",
    "TimedDifficultyAttributes",
    "TimingPoint",
    "Vector2",
    "__version__",
]
