from typing import Any, Final

from ._model import BeatmapDifficulty, BreakPeriod, HitObject, ScoreInfo, Slider, TimingPoint

version: Final[str]

class Beatmap:
    format_version: Final[int]
    mode: Final[int]
    stack_leniency: Final[float]
    difficulty: Final[BeatmapDifficulty]
    hit_objects: Final[list[HitObject]]
    sliders: Final[list[Slider]]
    timing_points: Final[list[TimingPoint]]
    breaks: Final[list[BreakPeriod]]

def calculate_difficulty(
    beatmap: Beatmap,
    mods: str | None,
    ruleset: int | None,
    clock_rate: float | None,
    /,
) -> dict[str, Any]: ...
def calculate_performance(
    beatmap: Beatmap,
    mods: str | None,
    state: ScoreInfo | None,
    combo: int | None,
    accuracy: float | None,
    misses: int | None,
    attributes: dict[str, Any] | None,
    /,
) -> dict[str, Any]: ...
def _record(name: str, fields: tuple[str, ...], /) -> type: ...
def _restore_record(cls: type, /) -> object: ...
def from_file(path: str | bytes, /) -> Beatmap: ...
