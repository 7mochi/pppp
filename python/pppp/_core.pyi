import os
from collections.abc import Mapping
from typing import Final

from ._model import (
    BeatmapDifficulty,
    BreakPeriod,
    DifficultyAttributes,
    HitObject,
    HitResult,
    PerformanceAttributes,
    Slider,
    Strains,
    TimedDifficultyAttributes,
    TimingPoint,
)

version: Final[str]

class Error(Exception): ...
class ModsError(Error, ValueError): ...
class ParseError(Error, ValueError): ...

class Beatmap:
    format_version: Final[int]
    mode: Final[int]
    stack_leniency: Final[float]
    difficulty: Final[BeatmapDifficulty]
    hit_objects: Final[list[HitObject]]
    sliders: Final[list[Slider]]
    timing_points: Final[list[TimingPoint]]
    breaks: Final[list[BreakPeriod]]
    @classmethod
    def from_bytes(cls, data: bytes | bytearray | memoryview, /) -> Beatmap: ...
    @classmethod
    def from_file(cls, path: str | bytes | os.PathLike[str] | os.PathLike[bytes], /) -> Beatmap: ...

def calculate_difficulty(
    beatmap: Beatmap,
    mods: str | int,
    ruleset: int | None,
    clock_rate: float | None,
    /,
) -> DifficultyAttributes: ...
def calculate_timed_difficulty(
    beatmap: Beatmap,
    mods: str | int,
    ruleset: int | None,
    clock_rate: float | None,
    /,
) -> list[TimedDifficultyAttributes]: ...
def calculate_strains(
    beatmap: Beatmap,
    mods: str | int,
    ruleset: int | None,
    clock_rate: float | None,
    /,
) -> Strains: ...
def calculate_performance(
    beatmap: Beatmap,
    mods: str | int,
    max_combo: int | None,
    accuracy: float | None,
    misses: int | None,
    statistics: Mapping[HitResult, int] | None,
    legacy_total_score: int | None,
    attributes: DifficultyAttributes | None,
    /,
) -> PerformanceAttributes: ...
def _record(name: str, fields: tuple[str, ...], /) -> type: ...
def _restore_record(cls: type, /) -> object: ...
