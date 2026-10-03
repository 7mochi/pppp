"""pppp: osu! difficulty and performance points, in native code."""

from __future__ import annotations

from enum import IntEnum

from . import _core
from ._core import Beatmap as Beatmap
from ._core import from_file as from_file
from ._core import version as version
from ._model import (
    BeatmapDifficulty,
    BreakPeriod,
    CatchDifficultyAttributes,
    CatchPerformanceAttributes,
    DifficultyAttributes,
    HitObject,
    ManiaDifficultyAttributes,
    ManiaPerformanceAttributes,
    OsuDifficultyAttributes,
    OsuPerformanceAttributes,
    PerformanceAttributes,
    ScoreInfo,
    Slider,
    SliderEvent,
    TaikoDifficultyAttributes,
    TaikoPerformanceAttributes,
    TimingPoint,
    Vector2,
    _difficulty_attributes,
    _performance_attributes,
)


class Ruleset(IntEnum):
    """The ruleset a tagged attribute set belongs to; the values are the beatmap's mode."""

    OSU = 0
    TAIKO = 1
    CATCH = 2
    MANIA = 3


class Difficulty:
    """Difficulty calculator on maps of any mode."""

    def __init__(
        self,
        beatmap: Beatmap,
        *,
        mods: str = "",
        ruleset: Ruleset | None = None,
        clock_rate: float | None = None,
    ) -> None:
        self._beatmap = beatmap
        self._mods = mods
        self._ruleset = ruleset
        self._clock_rate = clock_rate

    def mods(self, spec: str) -> Difficulty:
        """Specify mods, as osu!'s own specification list."""
        self._mods = spec
        return self

    def ruleset(self, ruleset: Ruleset) -> Difficulty:
        """Calculate for this ruleset instead of the beatmap's own mode."""
        self._ruleset = ruleset
        return self

    def clock_rate(self, rate: float) -> Difficulty:
        """Adjust the clock rate used in the calculation."""
        self._clock_rate = rate
        return self

    def calculate(self) -> DifficultyAttributes:
        """Perform the difficulty calculation."""
        return _difficulty_attributes(
            _core.calculate_difficulty(
                self._beatmap,
                self._mods,
                None if self._ruleset is None else int(self._ruleset),
                self._clock_rate,
            )
        )


class Performance:
    """Performance calculator on maps of any mode."""

    def __init__(
        self,
        beatmap: Beatmap,
        *,
        mods: str = "",
        state: ScoreInfo | None = None,
        combo: int | None = None,
        accuracy: float | None = None,
        misses: int | None = None,
        attributes: DifficultyAttributes | None = None,
    ) -> None:
        self._beatmap = beatmap
        self._mods = mods
        self._state = state
        self._combo = combo
        self._accuracy = accuracy
        self._misses = misses
        self._attributes = attributes

    def mods(self, spec: str) -> Performance:
        """Specify mods, as osu!'s own specification list."""
        self._mods = spec
        return self

    def state(self, state: ScoreInfo) -> Performance:
        """Provide the score state through a `ScoreInfo`."""
        self._state = state
        return self

    def combo(self, combo: int) -> Performance:
        """Specify the max combo of the play."""
        self._combo = combo
        return self

    def accuracy(self, accuracy: float) -> Performance:
        """Set the accuracy between 0.0 and 1.0."""
        self._accuracy = accuracy
        return self

    def misses(self, misses: int) -> Performance:
        """Specify the amount of misses of the play."""
        self._misses = misses
        return self

    def attributes(self, attributes: DifficultyAttributes) -> Performance:
        """Use the given already-calculated attributes, skipping the difficulty calculation."""
        self._attributes = attributes
        return self

    def _attributes_dict(self) -> dict | None:
        if self._attributes is None:
            return None
        if isinstance(self._attributes, dict):
            return self._attributes

        def convert(value: object) -> object:
            fields = getattr(value, "__dict__", None)
            if not fields:
                return value
            return {key: convert(item) for key, item in fields.items()}

        return convert(self._attributes)

    def calculate(self) -> PerformanceAttributes:
        """Perform the performance calculation, with the difficulty included unless attributes
        were given."""
        return _performance_attributes(
            _core.calculate_performance(
                self._beatmap,
                self._mods,
                self._state,
                self._combo,
                self._accuracy,
                self._misses,
                self._attributes_dict(),
            )
        )


__all__ = [
    "Beatmap",
    "BeatmapDifficulty",
    "BreakPeriod",
    "CatchDifficultyAttributes",
    "CatchPerformanceAttributes",
    "Difficulty",
    "DifficultyAttributes",
    "HitObject",
    "ManiaDifficultyAttributes",
    "ManiaPerformanceAttributes",
    "OsuDifficultyAttributes",
    "OsuPerformanceAttributes",
    "Performance",
    "PerformanceAttributes",
    "Ruleset",
    "ScoreInfo",
    "Slider",
    "SliderEvent",
    "TaikoDifficultyAttributes",
    "TaikoPerformanceAttributes",
    "TimingPoint",
    "Vector2",
    "from_file",
    "version",
]
