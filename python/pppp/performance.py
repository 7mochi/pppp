"""Performance calculation."""

from __future__ import annotations

import ctypes
from collections.abc import Mapping
from dataclasses import dataclass
from typing import Any, cast

from pppp._ctypes import _native
import pppp.mods as mods_io
from pppp.attributes import (
    CatchDifficultyAttributes,
    DifficultyAttributes,
    CatchPerformanceAttributes,
    ManiaDifficultyAttributes,
    ManiaPerformanceAttributes,
    OsuDifficultyAttributes,
    OsuPerformanceAttributes,
    PerformanceAttributes,
    TaikoDifficultyAttributes,
    TaikoPerformanceAttributes,
    catch_difficulty_to_native,
    mania_difficulty_to_native,
    osu_difficulty_to_native,
    taiko_difficulty_to_native,
)
from pppp.beatmap import Beatmap, require
from pppp.errors import Error
from pppp.hit_result import HitResult
from pppp.native import check_status
from pppp.ruleset import Ruleset

_COUNT_FIELDS = {
    int(HitResult.MISS): "count_miss",
    int(HitResult.MEH): "count_meh",
    int(HitResult.OK): "count_ok",
    int(HitResult.GOOD): "count_good",
    int(HitResult.GREAT): "count_great",
    int(HitResult.PERFECT): "count_perfect",
    int(HitResult.SMALL_TICK_MISS): "count_small_tick_miss",
    int(HitResult.SMALL_TICK_HIT): "count_small_tick_hit",
    int(HitResult.LARGE_TICK_MISS): "count_large_tick_miss",
    int(HitResult.LARGE_TICK_HIT): "count_large_tick_hit",
    int(HitResult.SLIDER_TAIL_HIT): "count_slider_tail_hit",
}


@dataclass(frozen=True, kw_only=True)
class Performance:
    """Settings for performance calculation."""

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
        """Calculate a score's performance."""
        beatmap = require(beatmap)
        if attributes is not None and not isinstance(
            attributes,
            (
                OsuDifficultyAttributes,
                TaikoDifficultyAttributes,
                CatchDifficultyAttributes,
                DifficultyAttributes,
                ManiaDifficultyAttributes,
            ),
        ):
            raise TypeError("expected difficulty attributes")

        ruleset = int(attributes.ruleset) if attributes is not None else beatmap.mode
        native_ruleset = _native.PpppNativeRuleset()
        check_status(_native.Pppp_Ruleset_CreateFromId(ruleset, ctypes.byref(native_ruleset)))

        calculator_type, create, calculate, destroy, output_type, convert = _calculator(ruleset)
        calculator = calculator_type()
        try:
            with mods_io.parse(self.mods, ruleset, self.classic) as mods:
                score = _native.PpppNativeScoreInfo()
                score.ruleset = native_ruleset.handle
                score.beatmap = beatmap.ensure_open().handle
                if mods:
                    score.mods = mods
                score.max_combo = int(self.max_combo) if self.max_combo is not None else 0
                score.accuracy = float(self.accuracy) if self.accuracy is not None else 0.0
                if self.legacy_total_score is not None:
                    score.legacy_total_score.has_value = 1
                    score.legacy_total_score.value = int(self.legacy_total_score)

                if self.statistics is not None:
                    if not isinstance(self.statistics, Mapping):  # pyright: ignore[reportUnnecessaryIsInstance]
                        raise TypeError("statistics must be a mapping")

                    for key, value in self.statistics.items():
                        if isinstance(key, bool) or not isinstance(key, int):  # type: ignore[unreachable]  # pyright: ignore[reportUnnecessaryIsInstance]
                            raise ValueError("statistics keys must be supported hit results")

                        field = _COUNT_FIELDS.get(int(key))
                        if field is None:
                            raise ValueError("statistics keys must be supported hit results")

                        setattr(score, field, int(value))
                if self.misses is not None:
                    score.count_miss = int(self.misses)

                native_attributes = (
                    _difficulty_to_native(attributes) if attributes is not None else None
                )
                check_status(
                    create(
                        ctypes.byref(calculator),
                    )
                )
                try:
                    output = output_type()
                    pointer = ctypes.byref(native_attributes) if native_attributes is not None else None
                    check_status(calculate(calculator.handle, ctypes.byref(score), pointer, ctypes.byref(output)))

                    return cast(PerformanceAttributes, convert(output))
                finally:
                    destroy(calculator.handle)
        finally:
            _native.Pppp_Ruleset_Destroy(native_ruleset.handle)


def _difficulty_to_native(attributes: Any) -> Any:
    if isinstance(attributes, OsuDifficultyAttributes):
        return osu_difficulty_to_native(attributes)

    if isinstance(attributes, TaikoDifficultyAttributes):
        return taiko_difficulty_to_native(attributes)

    if isinstance(attributes, CatchDifficultyAttributes):
        return catch_difficulty_to_native(attributes)

    if isinstance(attributes, ManiaDifficultyAttributes):
        return mania_difficulty_to_native(attributes)

    raise TypeError("expected difficulty attributes")


def _calculator(ruleset: int) -> tuple[Any, ...]:
    if ruleset == int(Ruleset.OSU):
        return (
            _native.PpppNativeOsuPerformanceCalculator,
            _native.Pppp_OsuPerformanceCalculator_Create,
            _native.Pppp_OsuPerformanceCalculator_Calculate,
            _native.Pppp_OsuPerformanceCalculator_Destroy,
            _native.PpppOsuPerformanceAttributes,
            _osu_from_native,
        )

    if ruleset == int(Ruleset.TAIKO):
        return (
            _native.PpppNativeTaikoPerformanceCalculator,
            _native.Pppp_TaikoPerformanceCalculator_Create,
            _native.Pppp_TaikoPerformanceCalculator_Calculate,
            _native.Pppp_TaikoPerformanceCalculator_Destroy,
            _native.PpppTaikoPerformanceAttributes,
            _taiko_from_native,
        )

    if ruleset == int(Ruleset.CATCH):
        return (
            _native.PpppNativeCatchPerformanceCalculator,
            _native.Pppp_CatchPerformanceCalculator_Create,
            _native.Pppp_CatchPerformanceCalculator_Calculate,
            _native.Pppp_CatchPerformanceCalculator_Destroy,
            _native.PpppCatchPerformanceAttributes,
            _catch_from_native,
        )

    if ruleset == int(Ruleset.MANIA):
        return (
            _native.PpppNativeManiaPerformanceCalculator,
            _native.Pppp_ManiaPerformanceCalculator_Create,
            _native.Pppp_ManiaPerformanceCalculator_Calculate,
            _native.Pppp_ManiaPerformanceCalculator_Destroy,
            _native.PpppManiaPerformanceAttributes,
            _mania_from_native,
        )

    raise Error(f"unknown ruleset {ruleset}")


def _optional(value: Any) -> float | None:
    return float(value.value) if value.has_value else None


def _osu_from_native(native: Any) -> OsuPerformanceAttributes:
    return OsuPerformanceAttributes(
        total=float(native.total),
        aim=float(native.aim),
        speed=float(native.speed),
        accuracy=float(native.accuracy),
        flashlight=float(native.flashlight),
        reading=float(native.reading),
        effective_miss_count=float(native.effective_miss_count),
        combo_based_estimated_miss_count=float(native.combo_based_estimated_miss_count),
        score_based_estimated_miss_count=_optional(native.score_based_estimated_miss_count),
        aim_estimated_slider_breaks=float(native.aim_estimated_slider_breaks),
        speed_estimated_slider_breaks=float(native.speed_estimated_slider_breaks),
        speed_deviation=_optional(native.speed_deviation),
    )


def _taiko_from_native(native: Any) -> TaikoPerformanceAttributes:
    return TaikoPerformanceAttributes(
        total=float(native.total),
        difficulty=float(native.difficulty),
        accuracy=float(native.accuracy),
        estimated_unstable_rate=_optional(native.estimated_unstable_rate),
    )


def _catch_from_native(native: Any) -> CatchPerformanceAttributes:
    return CatchPerformanceAttributes(total=float(native.total))


def _mania_from_native(native: Any) -> ManiaPerformanceAttributes:
    return ManiaPerformanceAttributes(
        total=float(native.total), difficulty=float(native.difficulty)
    )
