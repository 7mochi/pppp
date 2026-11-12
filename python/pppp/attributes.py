"""Difficulty and performance results."""

from __future__ import annotations

from dataclasses import dataclass, fields
from typing import Any, ClassVar, TypeAlias

from pppp._ctypes import _native
from pppp.ruleset import Ruleset


@dataclass(frozen=True)
class _DifficultyAttributesBase:
    star_rating: float
    max_combo: int


@dataclass(frozen=True)
class _PerformanceAttributesBase:
    total: float


@dataclass(frozen=True)
class OsuDifficultyAttributes(_DifficultyAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    aim_difficulty: float
    speed_difficulty: float
    reading_difficulty: float
    flashlight_difficulty: float
    slider_factor: float
    aim_difficult_strain_count: float
    speed_difficult_strain_count: float
    reading_difficult_note_count: float
    aim_difficult_slider_count: float
    aim_top_weighted_slider_factor: float
    speed_top_weighted_slider_factor: float
    speed_note_count: float
    hit_circle_count: int
    slider_count: int
    large_tick_count: int
    spinner_count: int
    nested_score_per_object: float
    legacy_score_base_multiplier: float
    maximum_legacy_combo_score: float


@dataclass(frozen=True)
class TaikoDifficultyAttributes(_DifficultyAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    mechanical_difficulty: float
    rhythm_difficulty: float
    reading_difficulty: float
    colour_difficulty: float
    stamina_difficulty: float
    mono_stamina_factor: float
    consistency_factor: float
    stamina_top_strains: float


@dataclass(frozen=True)
class CatchDifficultyAttributes(_DifficultyAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.CATCH


@dataclass(frozen=True)
class ManiaDifficultyAttributes(_DifficultyAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.MANIA


@dataclass(frozen=True)
class TimedDifficultyAttributes:
    time: float
    attributes: DifficultyAttributes


@dataclass(frozen=True)
class OsuPerformanceAttributes(_PerformanceAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    aim: float
    speed: float
    accuracy: float
    flashlight: float
    reading: float
    effective_miss_count: float
    combo_based_estimated_miss_count: float
    score_based_estimated_miss_count: float | None
    aim_estimated_slider_breaks: float
    speed_estimated_slider_breaks: float
    speed_deviation: float | None


@dataclass(frozen=True)
class TaikoPerformanceAttributes(_PerformanceAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    difficulty: float
    accuracy: float
    estimated_unstable_rate: float | None


@dataclass(frozen=True)
class CatchPerformanceAttributes(_PerformanceAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.CATCH


@dataclass(frozen=True)
class ManiaPerformanceAttributes(_PerformanceAttributesBase):
    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    difficulty: float


DifficultyAttributes: TypeAlias = (
    OsuDifficultyAttributes
    | TaikoDifficultyAttributes
    | CatchDifficultyAttributes
    | ManiaDifficultyAttributes
)

PerformanceAttributes: TypeAlias = (
    OsuPerformanceAttributes
    | TaikoPerformanceAttributes
    | CatchPerformanceAttributes
    | ManiaPerformanceAttributes
)


def osu_difficulty_from_native(native: Any) -> OsuDifficultyAttributes:
    return OsuDifficultyAttributes(
        star_rating=float(native.star_rating),
        max_combo=int(native.max_combo),
        aim_difficulty=float(native.aim_difficulty),
        speed_difficulty=float(native.speed_difficulty),
        reading_difficulty=float(native.reading_difficulty),
        flashlight_difficulty=float(native.flashlight_difficulty),
        slider_factor=float(native.slider_factor),
        aim_difficult_strain_count=float(native.aim_difficult_strain_count),
        speed_difficult_strain_count=float(native.speed_difficult_strain_count),
        reading_difficult_note_count=float(native.reading_difficult_note_count),
        aim_difficult_slider_count=float(native.aim_difficult_slider_count),
        aim_top_weighted_slider_factor=float(native.aim_top_weighted_slider_factor),
        speed_top_weighted_slider_factor=float(native.speed_top_weighted_slider_factor),
        speed_note_count=float(native.speed_note_count),
        hit_circle_count=int(native.hit_circle_count),
        slider_count=int(native.slider_count),
        large_tick_count=int(native.large_tick_count),
        spinner_count=int(native.spinner_count),
        nested_score_per_object=float(native.nested_score_per_object),
        legacy_score_base_multiplier=float(native.legacy_score_base_multiplier),
        maximum_legacy_combo_score=float(native.maximum_legacy_combo_score),
    )


def taiko_difficulty_from_native(native: Any) -> TaikoDifficultyAttributes:
    return TaikoDifficultyAttributes(
        star_rating=float(native.star_rating),
        max_combo=int(native.max_combo),
        mechanical_difficulty=float(native.mechanical_difficulty),
        rhythm_difficulty=float(native.rhythm_difficulty),
        reading_difficulty=float(native.reading_difficulty),
        colour_difficulty=float(native.colour_difficulty),
        stamina_difficulty=float(native.stamina_difficulty),
        mono_stamina_factor=float(native.mono_stamina_factor),
        consistency_factor=float(native.consistency_factor),
        stamina_top_strains=float(native.stamina_top_strains),
    )


def catch_difficulty_from_native(native: Any) -> CatchDifficultyAttributes:
    return CatchDifficultyAttributes(
        star_rating=float(native.star_rating), max_combo=int(native.max_combo)
    )


def mania_difficulty_from_native(native: Any) -> ManiaDifficultyAttributes:
    return ManiaDifficultyAttributes(
        star_rating=float(native.star_rating), max_combo=int(native.max_combo)
    )


def _require_numbers(attributes: Any) -> None:
    for attribute in fields(attributes):
        if not isinstance(getattr(attributes, attribute.name), (int, float)):
            raise TypeError(f"{attribute.name} must be a number")


def osu_difficulty_to_native(attributes: OsuDifficultyAttributes) -> object:
    _require_numbers(attributes)
    native = _native.PpppOsuDifficultyAttributes()
    for name in (
        "star_rating",
        "max_combo",
        "aim_difficulty",
        "speed_difficulty",
        "reading_difficulty",
        "flashlight_difficulty",
        "slider_factor",
        "aim_difficult_strain_count",
        "speed_difficult_strain_count",
        "reading_difficult_note_count",
        "aim_difficult_slider_count",
        "aim_top_weighted_slider_factor",
        "speed_top_weighted_slider_factor",
        "speed_note_count",
        "hit_circle_count",
        "slider_count",
        "large_tick_count",
        "spinner_count",
        "nested_score_per_object",
        "legacy_score_base_multiplier",
        "maximum_legacy_combo_score",
    ):
        setattr(native, name, getattr(attributes, name))

    return native


def taiko_difficulty_to_native(attributes: TaikoDifficultyAttributes) -> object:
    _require_numbers(attributes)
    native = _native.PpppTaikoDifficultyAttributes()
    for name in (
        "star_rating",
        "max_combo",
        "mechanical_difficulty",
        "rhythm_difficulty",
        "reading_difficulty",
        "colour_difficulty",
        "stamina_difficulty",
        "mono_stamina_factor",
        "consistency_factor",
        "stamina_top_strains",
    ):
        setattr(native, name, getattr(attributes, name))

    return native


def catch_difficulty_to_native(attributes: CatchDifficultyAttributes) -> object:
    _require_numbers(attributes)
    native = _native.PpppCatchDifficultyAttributes()
    native.star_rating = attributes.star_rating
    native.max_combo = attributes.max_combo

    return native


def mania_difficulty_to_native(attributes: ManiaDifficultyAttributes) -> object:
    _require_numbers(attributes)
    native = _native.PpppManiaDifficultyAttributes()
    native.star_rating = attributes.star_rating
    native.max_combo = attributes.max_combo

    return native
