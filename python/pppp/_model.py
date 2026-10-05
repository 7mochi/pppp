"""The value types the extension builds, declared here and given native storage by `_core`."""

from __future__ import annotations

import sys
from enum import IntEnum
from typing import ClassVar, TypeAlias, TypeVar, cast, get_origin

if sys.version_info >= (3, 11):
    from typing import dataclass_transform
else:
    from typing_extensions import dataclass_transform

from . import _core

_T = TypeVar("_T")


def _restore(cls: type[_T]) -> _T:
    # Pickle/deepcopy memoize the empty record before restoring cyclic fields.
    return cast(_T, _core._restore_record(cls))


@dataclass_transform(frozen_default=True)
def _record(cls: type[_T]) -> type[_T]:
    """Keep the domain declaration here; give its eager fields native storage."""
    annotations: dict[str, object] = {}
    for base in reversed(cls.__mro__[:-1]):
        annotations.update(base.__annotations__)
    fields = tuple(
        name
        for name, hint in annotations.items()
        if cast(object, get_origin(hint)) is not ClassVar
        and not (isinstance(hint, str) and hint.startswith("ClassVar["))
    )
    result = _core._record(cls.__name__, fields)
    for name, value in vars(cls).items():
        if name not in {"__dict__", "__weakref__", "__annotations__"}:
            setattr(result, name, value)
    result.__annotations__ = annotations
    setattr(result, "__match_args__", fields)
    return cast(type[_T], result)


class Ruleset(IntEnum):
    """Which ruleset a tagged attribute set belongs to. The values are the beatmap's `Mode`."""

    OSU = 0
    TAIKO = 1
    CATCH = 2
    MANIA = 3


class HitResult(IntEnum):
    NONE = 0
    MISS = 1
    MEH = 2
    OK = 3
    GOOD = 4
    GREAT = 5
    PERFECT = 6
    SMALL_TICK_MISS = 7
    SMALL_TICK_HIT = 8
    LARGE_TICK_MISS = 9
    LARGE_TICK_HIT = 10
    SMALL_BONUS = 11
    LARGE_BONUS = 12
    IGNORE_MISS = 13
    IGNORE_HIT = 14
    COMBO_BREAK = 15
    SLIDER_TAIL_HIT = 16
    LEGACY_COMBO_INCREASE = 17


@_record
class Vector2:
    x: float
    y: float


@_record
class BeatmapDifficulty:
    drain_rate: float
    circle_size: float
    overall_difficulty: float
    approach_rate: float
    slider_multiplier: float
    slider_tick_rate: float


@_record
class HitObject:
    position: Vector2
    type: int
    hitsound: int
    start_time: float
    end_time: float
    new_combo: bool
    combo_offset: int
    slider: int


@_record
class SliderEvent:
    type: int
    time: float
    span_index: int
    span_start_time: float
    path_progress: float
    position: Vector2


@_record
class Slider:
    slides: int
    expected_length: float
    node_sounds: list[int]
    control_points: list[Vector2]
    path: list[Vector2]
    cumulative_lengths: list[float]
    undecimated_path: list[Vector2]
    undecimated_cumulative_lengths: list[float]
    events: list[SliderEvent]
    catch_events: list[SliderEvent]


@_record
class TimingPoint:
    time: float
    beat_length: float
    meter: int
    uninherited: bool
    effects: int


@_record
class BreakPeriod:
    start_time: float
    end_time: float


@_record
class OsuDifficultyAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    star_rating: float
    max_combo: int
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


@_record
class TaikoDifficultyAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    star_rating: float
    max_combo: int
    mechanical_difficulty: float
    rhythm_difficulty: float
    reading_difficulty: float
    colour_difficulty: float
    stamina_difficulty: float
    mono_stamina_factor: float
    consistency_factor: float
    stamina_top_strains: float


@_record
class CatchDifficultyAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.CATCH
    star_rating: float
    max_combo: int


@_record
class ManiaDifficultyAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    star_rating: float
    max_combo: int


DifficultyAttributes: TypeAlias = (
    OsuDifficultyAttributes
    | TaikoDifficultyAttributes
    | CatchDifficultyAttributes
    | ManiaDifficultyAttributes
)


@_record
class TimedDifficultyAttributes:
    """Wraps a DifficultyAttributes object and adds a time value for which the attribute is valid.
    Output by `Difficulty.calculate_timed`."""

    time: float
    attributes: DifficultyAttributes


@_record
class OsuStrains:
    """The result of calculating the strains on a osu! map.

    Suitable to plot the difficulty of a map over time."""

    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    start_time: float
    section_length: float
    aim: list[float]
    aim_no_sliders: list[float]
    speed: list[float]
    reading: list[float]
    flashlight: list[float]


@_record
class TaikoStrains:
    """The result of calculating the strains on a osu!taiko map.

    Suitable to plot the difficulty of a map over time."""

    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    start_time: float
    section_length: float
    colour: list[float]
    reading: list[float]
    rhythm: list[float]
    stamina: list[float]
    single_colour_stamina: list[float]


@_record
class CatchStrains:
    """The result of calculating the strains on a osu!catch map.

    Suitable to plot the difficulty of a map over time."""

    ruleset: ClassVar[Ruleset] = Ruleset.CATCH
    start_time: float
    section_length: float
    movement: list[float]


@_record
class ManiaStrains:
    """The result of calculating the strains on a osu!mania map.

    Suitable to plot the difficulty of a map over time."""

    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    start_time: float
    section_length: float
    strain: list[float]


Strains: TypeAlias = OsuStrains | TaikoStrains | CatchStrains | ManiaStrains


@_record
class OsuPerformanceAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    total: float
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


@_record
class TaikoPerformanceAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    total: float
    difficulty: float
    accuracy: float
    estimated_unstable_rate: float | None


@_record
class CatchPerformanceAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.CATCH
    total: float


@_record
class ManiaPerformanceAttributes:
    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    total: float
    difficulty: float


PerformanceAttributes: TypeAlias = (
    OsuPerformanceAttributes
    | TaikoPerformanceAttributes
    | CatchPerformanceAttributes
    | ManiaPerformanceAttributes
)
