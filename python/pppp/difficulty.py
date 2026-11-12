"""Difficulty calculation."""

from __future__ import annotations

import ctypes
from dataclasses import dataclass
from typing import Any, cast

from pppp._ctypes import _native
import pppp.mods as mods_io
from pppp.attributes import (
    DifficultyAttributes,
    catch_difficulty_from_native,
    mania_difficulty_from_native,
    osu_difficulty_from_native,
    taiko_difficulty_from_native,
)
from pppp.attributes import TimedDifficultyAttributes
from pppp.beatmap import Beatmap, require
from pppp.errors import Error
from pppp.native import check_status, read_doubles
from pppp.ruleset import Ruleset
from pppp.strains import CatchStrains, ManiaStrains, OsuStrains, Strains, TaikoStrains


@dataclass(frozen=True, kw_only=True)
class Difficulty:
    """Settings for difficulty calculation."""

    mods: str | int = ""
    ruleset: Ruleset | int | None = None
    clock_rate: float | None = None
    classic: bool = False

    def calculate(self, beatmap: Beatmap) -> DifficultyAttributes:
        """Calculate a beatmap's difficulty."""
        beatmap = require(beatmap)
        ruleset = int(self.ruleset) if self.ruleset is not None else beatmap.mode
        native_ruleset = _native.PpppNativeRuleset()
        check_status(_native.Pppp_Ruleset_CreateFromId(ruleset, ctypes.byref(native_ruleset)))

        calculator_type, create, calculate, destroy, set_clock_rate, output_type, convert = (
            _calculator(ruleset)
        )
        calculator = calculator_type()
        try:
            with mods_io.parse(self.mods, ruleset, self.classic) as mods:
                check_status(
                    create(
                        native_ruleset.handle,
                        beatmap.ensure_open().handle,
                        ctypes.byref(calculator),
                    )
                )

                try:
                    if self.clock_rate is not None:
                        check_status(set_clock_rate(calculator.handle, float(self.clock_rate)))
                    output = output_type()
                    check_status(calculate(calculator.handle, mods, ctypes.byref(output)))

                    return cast(DifficultyAttributes, convert(output))
                finally:
                    destroy(calculator.handle)
        finally:
            _native.Pppp_Ruleset_Destroy(native_ruleset.handle)

    def calculate_timed(self, beatmap: Beatmap) -> list[TimedDifficultyAttributes]:
        """Calculate difficulty over time."""
        beatmap = require(beatmap)
        ruleset = int(self.ruleset) if self.ruleset is not None else beatmap.mode
        native_ruleset = _native.PpppNativeRuleset()
        check_status(_native.Pppp_Ruleset_CreateFromId(ruleset, ctypes.byref(native_ruleset)))

        calculator_type, create, timed, destroy, set_clock_rate, getter, entry_type, convert = (
            _timed(ruleset)
        )
        calculator = calculator_type()
        result = ctypes.POINTER(_native.PpppTimedResult)()
        try:
            with mods_io.parse(self.mods, ruleset, self.classic) as mods:
                check_status(
                    create(
                        native_ruleset.handle,
                        beatmap.ensure_open().handle,
                        ctypes.byref(calculator),
                    )
                )

                try:
                    if self.clock_rate is not None:
                        check_status(set_clock_rate(calculator.handle, float(self.clock_rate)))
                    check_status(timed(calculator.handle, mods, ctypes.byref(result)))
                finally:
                    destroy(calculator.handle)

            count = _native.PpppInt32(0)
            check_status(_native.Pppp_TimedResult_GetCount(result, ctypes.byref(count)))
            if count.value == 0:
                return []

            entries = (entry_type * count.value)()
            size = _native.PpppInt32(count.value)
            check_status(getter(result, entries, ctypes.byref(size)))

            return [
                TimedDifficultyAttributes(
                    time=float(entry.time), attributes=convert(entry.attributes)
                )
                for entry in entries
            ]
        finally:
            if bool(result):
                _native.Pppp_TimedResult_Destroy(result)
            _native.Pppp_Ruleset_Destroy(native_ruleset.handle)

    def calculate_strains(self, beatmap: Beatmap) -> Strains:
        """Calculate a beatmap's strain series."""
        beatmap = require(beatmap)
        ruleset = int(self.ruleset) if self.ruleset is not None else beatmap.mode
        native_ruleset = _native.PpppNativeRuleset()
        check_status(_native.Pppp_Ruleset_CreateFromId(ruleset, ctypes.byref(native_ruleset)))

        calculator_type, create, calculate, destroy, set_clock_rate = _strain_calculator(ruleset)
        calculator = calculator_type()
        result = ctypes.POINTER(_native.PpppStrains)()
        try:
            with mods_io.parse(self.mods, ruleset, self.classic) as mods:
                check_status(
                    create(
                        native_ruleset.handle,
                        beatmap.ensure_open().handle,
                        ctypes.byref(calculator),
                    )
                )

                try:
                    if self.clock_rate is not None:
                        check_status(set_clock_rate(calculator.handle, float(self.clock_rate)))
                    check_status(calculate(calculator.handle, mods, ctypes.byref(result)))
                finally:
                    destroy(calculator.handle)

            native_id = _native.PpppInt32()
            start = ctypes.c_double()
            length = ctypes.c_double()
            check_status(_native.Pppp_Strains_GetRuleset(result, ctypes.byref(native_id)))
            check_status(_native.Pppp_Strains_GetStartTime(result, ctypes.byref(start)))
            check_status(_native.Pppp_Strains_GetSectionLength(result, ctypes.byref(length)))
            common = {"start_time": float(start.value), "section_length": float(length.value)}
            if native_id.value == int(Ruleset.OSU):
                return OsuStrains(
                    **common,
                    aim=read_doubles(_native.Pppp_Strains_GetOsuAim, result),
                    aim_no_sliders=read_doubles(_native.Pppp_Strains_GetOsuAimNoSliders, result),
                    speed=read_doubles(_native.Pppp_Strains_GetOsuSpeed, result),
                    reading=read_doubles(_native.Pppp_Strains_GetOsuReading, result),
                    flashlight=read_doubles(_native.Pppp_Strains_GetOsuFlashlight, result),
                )

            if native_id.value == int(Ruleset.TAIKO):
                return TaikoStrains(
                    **common,
                    colour=read_doubles(_native.Pppp_Strains_GetTaikoColour, result),
                    reading=read_doubles(_native.Pppp_Strains_GetTaikoReading, result),
                    rhythm=read_doubles(_native.Pppp_Strains_GetTaikoRhythm, result),
                    stamina=read_doubles(_native.Pppp_Strains_GetTaikoStamina, result),
                    single_colour_stamina=read_doubles(
                        _native.Pppp_Strains_GetTaikoSingleColourStamina, result
                    ),
                )

            if native_id.value == int(Ruleset.CATCH):
                return CatchStrains(
                    **common, movement=read_doubles(_native.Pppp_Strains_GetCatchMovement, result)
                )

            if native_id.value == int(Ruleset.MANIA):
                return ManiaStrains(**common, strain=read_doubles(_native.Pppp_Strains_GetManiaStrain, result))

            raise Error(f"unknown ruleset {native_id.value}")
        finally:
            if bool(result):
                _native.Pppp_Strains_Destroy(result)
            _native.Pppp_Ruleset_Destroy(native_ruleset.handle)


def _calculator(ruleset: int) -> tuple[Any, ...]:
    if ruleset == int(Ruleset.OSU):
        return (
            _native.PpppNativeOsuDifficultyCalculator,
            _native.Pppp_OsuDifficultyCalculator_Create,
            _native.Pppp_OsuDifficultyCalculator_Calculate,
            _native.Pppp_OsuDifficultyCalculator_Destroy,
            _native.Pppp_OsuDifficultyCalculator_SetClockRate,
            _native.PpppOsuDifficultyAttributes,
            osu_difficulty_from_native,
        )

    if ruleset == int(Ruleset.TAIKO):
        return (
            _native.PpppNativeTaikoDifficultyCalculator,
            _native.Pppp_TaikoDifficultyCalculator_Create,
            _native.Pppp_TaikoDifficultyCalculator_Calculate,
            _native.Pppp_TaikoDifficultyCalculator_Destroy,
            _native.Pppp_TaikoDifficultyCalculator_SetClockRate,
            _native.PpppTaikoDifficultyAttributes,
            taiko_difficulty_from_native,
        )

    if ruleset == int(Ruleset.CATCH):
        return (
            _native.PpppNativeCatchDifficultyCalculator,
            _native.Pppp_CatchDifficultyCalculator_Create,
            _native.Pppp_CatchDifficultyCalculator_Calculate,
            _native.Pppp_CatchDifficultyCalculator_Destroy,
            _native.Pppp_CatchDifficultyCalculator_SetClockRate,
            _native.PpppCatchDifficultyAttributes,
            catch_difficulty_from_native,
        )

    if ruleset == int(Ruleset.MANIA):
        return (
            _native.PpppNativeManiaDifficultyCalculator,
            _native.Pppp_ManiaDifficultyCalculator_Create,
            _native.Pppp_ManiaDifficultyCalculator_Calculate,
            _native.Pppp_ManiaDifficultyCalculator_Destroy,
            _native.Pppp_ManiaDifficultyCalculator_SetClockRate,
            _native.PpppManiaDifficultyAttributes,
            mania_difficulty_from_native,
        )

    raise Error(f"unknown ruleset {ruleset}")

def _timed(ruleset: int) -> tuple[Any, ...]:
    if ruleset == int(Ruleset.OSU):
        return (
            _native.PpppNativeOsuDifficultyCalculator,
            _native.Pppp_OsuDifficultyCalculator_Create,
            _native.Pppp_OsuDifficultyCalculator_CalculateTimed,
            _native.Pppp_OsuDifficultyCalculator_Destroy,
            _native.Pppp_OsuDifficultyCalculator_SetClockRate,
            _native.Pppp_TimedResult_GetOsu,
            _native.PpppTimedOsuDifficultyAttributes,
            osu_difficulty_from_native,
        )

    if ruleset == int(Ruleset.TAIKO):
        return (
            _native.PpppNativeTaikoDifficultyCalculator,
            _native.Pppp_TaikoDifficultyCalculator_Create,
            _native.Pppp_TaikoDifficultyCalculator_CalculateTimed,
            _native.Pppp_TaikoDifficultyCalculator_Destroy,
            _native.Pppp_TaikoDifficultyCalculator_SetClockRate,
            _native.Pppp_TimedResult_GetTaiko,
            _native.PpppTimedTaikoDifficultyAttributes,
            taiko_difficulty_from_native,
        )

    if ruleset == int(Ruleset.CATCH):
        return (
            _native.PpppNativeCatchDifficultyCalculator,
            _native.Pppp_CatchDifficultyCalculator_Create,
            _native.Pppp_CatchDifficultyCalculator_CalculateTimed,
            _native.Pppp_CatchDifficultyCalculator_Destroy,
            _native.Pppp_CatchDifficultyCalculator_SetClockRate,
            _native.Pppp_TimedResult_GetCatch,
            _native.PpppTimedCatchDifficultyAttributes,
            catch_difficulty_from_native,
        )

    if ruleset == int(Ruleset.MANIA):
        return (
            _native.PpppNativeManiaDifficultyCalculator,
            _native.Pppp_ManiaDifficultyCalculator_Create,
            _native.Pppp_ManiaDifficultyCalculator_CalculateTimed,
            _native.Pppp_ManiaDifficultyCalculator_Destroy,
            _native.Pppp_ManiaDifficultyCalculator_SetClockRate,
            _native.Pppp_TimedResult_GetMania,
            _native.PpppTimedManiaDifficultyAttributes,
            mania_difficulty_from_native,
        )

    raise Error(f"unknown ruleset {ruleset}")

def _strain_calculator(ruleset: int) -> tuple[Any, ...]:
    if ruleset == int(Ruleset.OSU):
        return (
            _native.PpppNativeOsuDifficultyCalculator,
            _native.Pppp_OsuDifficultyCalculator_Create,
            _native.Pppp_OsuDifficultyCalculator_CalculateStrains,
            _native.Pppp_OsuDifficultyCalculator_Destroy,
            _native.Pppp_OsuDifficultyCalculator_SetClockRate,
        )

    if ruleset == int(Ruleset.TAIKO):
        return (
            _native.PpppNativeTaikoDifficultyCalculator,
            _native.Pppp_TaikoDifficultyCalculator_Create,
            _native.Pppp_TaikoDifficultyCalculator_CalculateStrains,
            _native.Pppp_TaikoDifficultyCalculator_Destroy,
            _native.Pppp_TaikoDifficultyCalculator_SetClockRate,
        )

    if ruleset == int(Ruleset.CATCH):
        return (
            _native.PpppNativeCatchDifficultyCalculator,
            _native.Pppp_CatchDifficultyCalculator_Create,
            _native.Pppp_CatchDifficultyCalculator_CalculateStrains,
            _native.Pppp_CatchDifficultyCalculator_Destroy,
            _native.Pppp_CatchDifficultyCalculator_SetClockRate,
        )

    if ruleset == int(Ruleset.MANIA):
        return (
            _native.PpppNativeManiaDifficultyCalculator,
            _native.Pppp_ManiaDifficultyCalculator_Create,
            _native.Pppp_ManiaDifficultyCalculator_CalculateStrains,
            _native.Pppp_ManiaDifficultyCalculator_Destroy,
            _native.Pppp_ManiaDifficultyCalculator_SetClockRate,
        )

    raise Error(f"unknown ruleset {ruleset}")
