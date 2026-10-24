"""The difficulty calculator and the conversion of its output."""

from __future__ import annotations

import ctypes
from dataclasses import dataclass

from . import _native
from . import mods as mods_io
from .attributes import (
    CatchDifficultyAttributes,
    DifficultyAttributes,
    ManiaDifficultyAttributes,
    OsuDifficultyAttributes,
    TaikoDifficultyAttributes,
    TimedDifficultyAttributes,
)
from .beatmap import Beatmap, require
from .data import doubles
from .errors import Error
from .native import check, difficulty_options
from .ruleset import Ruleset
from .strains import CatchStrains, ManiaStrains, OsuStrains, Strains, TaikoStrains


def from_native(native: _native.pppp_difficulty_attributes) -> DifficultyAttributes:
    """Read the ruleset tag of a `pppp_difficulty_attributes` and convert the matching attributes."""
    if native.ruleset == int(Ruleset.OSU):
        return OsuDifficultyAttributes.from_native(native)
    if native.ruleset == int(Ruleset.TAIKO):
        return TaikoDifficultyAttributes.from_native(native)
    if native.ruleset == int(Ruleset.CATCH):
        return CatchDifficultyAttributes.from_native(native)
    if native.ruleset == int(Ruleset.MANIA):
        return ManiaDifficultyAttributes.from_native(native)
    raise Error(f"unknown ruleset {native.ruleset}")


@dataclass(frozen=True, kw_only=True)
class Difficulty:
    """Difficulty calculator on maps of any mode."""

    mods: str | int = ""
    ruleset: Ruleset | None = None
    clock_rate: float | None = None
    classic: bool = False

    def calculate(self, beatmap: Beatmap) -> DifficultyAttributes:
        """Perform the difficulty calculation."""
        beatmap = require(beatmap)
        ruleset = int(self.ruleset) if self.ruleset is not None else beatmap.mode
        with mods_io.parse(self.mods, ruleset, self.classic) as mods:
            options = difficulty_options(mods, self.ruleset, self.clock_rate)
            out = _native.pppp_difficulty_attributes()
            check(
                _native.pppp_calculate_difficulty(
                    beatmap._handle, ctypes.byref(options), ctypes.byref(out)
                )
            )
        return from_native(out)

    def calculate_timed(self, beatmap: Beatmap) -> list[TimedDifficultyAttributes]:
        """Calculates the difficulty at every relevant time value in the beatmap."""
        beatmap = require(beatmap)
        ruleset = int(self.ruleset) if self.ruleset is not None else beatmap.mode
        with mods_io.parse(self.mods, ruleset, self.classic) as mods:
            options = difficulty_options(mods, self.ruleset, self.clock_rate)
            timed = ctypes.POINTER(_native.pppp_timed_difficulty)()
            check(
                _native.pppp_calculate_timed_difficulty(
                    beatmap._handle, ctypes.byref(options), ctypes.byref(timed)
                )
            )
            try:
                entries = ctypes.POINTER(_native.pppp_timed_difficulty_attributes)()
                count = ctypes.c_size_t()
                check(
                    _native.pppp_timed_difficulty_entries(
                        timed, ctypes.byref(entries), ctypes.byref(count)
                    )
                )
                return [
                    TimedDifficultyAttributes(
                        time=float(entries[index].time),
                        attributes=from_native(entries[index].attributes),
                    )
                    for index in range(int(count.value))
                ]
            finally:
                _native.pppp_timed_difficulty_free(timed)

    def calculate_strains(self, beatmap: Beatmap) -> Strains:
        """Perform the difficulty calculation but return the strain series."""
        beatmap = require(beatmap)
        ruleset = int(self.ruleset) if self.ruleset is not None else beatmap.mode
        with mods_io.parse(self.mods, ruleset, self.classic) as mods:
            options = difficulty_options(mods, self.ruleset, self.clock_rate)
            strains = ctypes.POINTER(_native.pppp_strains)()
            check(
                _native.pppp_calculate_strains(
                    beatmap._handle, ctypes.byref(options), ctypes.byref(strains)
                )
            )
            try:
                native_ruleset = ctypes.c_int32()
                start_time = ctypes.c_double()
                section_length = ctypes.c_double()
                check(
                    _native.pppp_strains_info(
                        strains,
                        ctypes.byref(native_ruleset),
                        ctypes.byref(start_time),
                        ctypes.byref(section_length),
                    )
                )
                start = float(start_time.value)
                length = float(section_length.value)
                if native_ruleset.value == int(Ruleset.OSU):
                    osu = _native.pppp_osu_strains()
                    check(_native.pppp_strains_osu(strains, ctypes.byref(osu)))
                    return OsuStrains(
                        start_time=start,
                        section_length=length,
                        aim=doubles(osu.aim, osu.aim_count),
                        aim_no_sliders=doubles(osu.aim_no_sliders, osu.aim_no_sliders_count),
                        speed=doubles(osu.speed, osu.speed_count),
                        reading=doubles(osu.reading, osu.reading_count),
                        flashlight=doubles(osu.flashlight, osu.flashlight_count),
                    )
                if native_ruleset.value == int(Ruleset.TAIKO):
                    taiko = _native.pppp_taiko_strains()
                    check(_native.pppp_strains_taiko(strains, ctypes.byref(taiko)))
                    return TaikoStrains(
                        start_time=start,
                        section_length=length,
                        colour=doubles(taiko.colour, taiko.colour_count),
                        reading=doubles(taiko.reading, taiko.reading_count),
                        rhythm=doubles(taiko.rhythm, taiko.rhythm_count),
                        stamina=doubles(taiko.stamina, taiko.stamina_count),
                        single_colour_stamina=doubles(
                            taiko.single_colour_stamina, taiko.single_colour_stamina_count
                        ),
                    )
                if native_ruleset.value == int(Ruleset.CATCH):
                    catch = _native.pppp_catch_strains()
                    check(_native.pppp_strains_catch(strains, ctypes.byref(catch)))
                    return CatchStrains(
                        start_time=start,
                        section_length=length,
                        movement=doubles(catch.movement, catch.movement_count),
                    )
                if native_ruleset.value == int(Ruleset.MANIA):
                    mania = _native.pppp_mania_strains()
                    check(_native.pppp_strains_mania(strains, ctypes.byref(mania)))
                    return ManiaStrains(
                        start_time=start,
                        section_length=length,
                        strain=doubles(mania.strain, mania.strain_count),
                    )
                raise Error(f"unknown ruleset {native_ruleset.value}")
            finally:
                _native.pppp_strains_free(strains)
