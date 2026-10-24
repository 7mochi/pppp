"""The performance calculator and the conversion of its output."""

from __future__ import annotations

import ctypes
from collections.abc import Mapping
from dataclasses import dataclass

from . import _native
from . import mods as mods_io
from .attributes import (
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
)
from .beatmap import Beatmap, require
from .errors import Error
from .hit_result import HitResult
from .library import PPPP_HIT_RESULT_COUNT, PPPP_PERFORMANCE_OPTIONS_VERSION
from .native import check
from .ruleset import Ruleset


def from_native(native: _native.pppp_performance_attributes) -> PerformanceAttributes:
    """Read the ruleset tag of a `pppp_performance_attributes` and convert the matching attributes."""
    if native.ruleset == int(Ruleset.OSU):
        return OsuPerformanceAttributes.from_native(native)
    if native.ruleset == int(Ruleset.TAIKO):
        return TaikoPerformanceAttributes.from_native(native)
    if native.ruleset == int(Ruleset.CATCH):
        return CatchPerformanceAttributes.from_native(native)
    if native.ruleset == int(Ruleset.MANIA):
        return ManiaPerformanceAttributes.from_native(native)
    raise Error(f"unknown ruleset {native.ruleset}")


@dataclass(frozen=True, kw_only=True)
class Performance:
    """Performance calculator on maps of any mode."""

    mods: str | int = ""
    max_combo: int | None = None
    accuracy: float | None = None
    misses: int | None = None
    statistics: Mapping[HitResult, int] | None = None
    legacy_total_score: int | None = None
    classic: bool = False

    def calculate(
        self, beatmap: Beatmap, attributes: DifficultyAttributes | None = None
    ) -> PerformanceAttributes:
        """Perform the performance calculation for the map's or the attributes' mode."""
        beatmap = require(beatmap)
        provided: _native.pppp_difficulty_attributes | None = None
        if attributes is not None:
            if not isinstance(
                attributes,
                (
                    OsuDifficultyAttributes,
                    TaikoDifficultyAttributes,
                    CatchDifficultyAttributes,
                    ManiaDifficultyAttributes,
                ),
            ):
                raise TypeError("expected difficulty attributes")
            provided = attributes.to_native()

        ruleset = int(provided.ruleset) if provided is not None else beatmap.mode
        with mods_io.parse(self.mods, ruleset, self.classic) as mods:
            options = _native.pppp_performance_options()
            options.version = PPPP_PERFORMANCE_OPTIONS_VERSION
            options.mods = mods
            if provided is not None:
                options.difficulty = ctypes.pointer(provided)
                options.has_difficulty = 1
            if self.statistics is not None:
                if not isinstance(self.statistics, Mapping):
                    raise TypeError("statistics must be a mapping")
                for key, value in self.statistics.items():
                    index = int(key)
                    if index < 0 or index >= PPPP_HIT_RESULT_COUNT:
                        raise ValueError("statistics keys must be hit results")
                    options.score.statistics[index] = int(value)
                options.has_score = 1
            if self.legacy_total_score is not None:
                options.score.legacy_total_score = int(self.legacy_total_score)
                options.score.has_legacy_total_score = 1
                options.has_score = 1
            if self.max_combo is not None:
                options.combo = int(self.max_combo)
                options.has_combo = 1
            if self.accuracy is not None:
                options.accuracy = float(self.accuracy)
                options.has_accuracy = 1
            if self.misses is not None:
                options.misses = int(self.misses)
                options.has_misses = 1

            out = _native.pppp_performance_attributes()
            check(
                _native.pppp_calculate_performance(
                    beatmap._handle, ctypes.byref(options), ctypes.byref(out)
                )
            )
        return from_native(out)
