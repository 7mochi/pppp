"""The public surface as a typed user would write it; mypy checks this file."""

from pppp import (
    Beatmap,
    BeatmapDifficulty,
    BreakPeriod,
    Difficulty,
    DifficultyAttributes,
    HitObject,
    HitResult,
    OsuDifficultyAttributes,
    OsuStrains,
    Performance,
    PerformanceAttributes,
    Ruleset,
    Slider,
    Strains,
    TimingPoint,
    Vector2,
)


def origin() -> Vector2:
    return Vector2(0.0, 0.0)


def distance_squared(a: Vector2, b: Vector2) -> float:
    dx = a.x - b.x
    dy = a.y - b.y
    return dx * dx + dy * dy


def settings(difficulty: BeatmapDifficulty) -> tuple[float, float]:
    return difficulty.approach_rate, difficulty.overall_difficulty


def slider_length(slider: Slider) -> float:
    return slider.expected_length


def first_object(objects: list[HitObject]) -> float:
    return objects[0].start_time


def breaks_of(periods: list[BreakPeriod]) -> list[float]:
    return [period.start_time for period in periods]


def timing_of(points: list[TimingPoint]) -> list[float]:
    return [point.beat_length for point in points if point.uninherited]


def load(path: str) -> Beatmap:
    return Beatmap.from_file(path)


def stars(beatmap: Beatmap) -> float:
    attributes: DifficultyAttributes = Difficulty(mods="HD,DT").calculate(beatmap)
    return attributes.star_rating


def aim(beatmap: Beatmap) -> float | None:
    match Difficulty(mods=72, ruleset=Ruleset.OSU).calculate(beatmap):
        case OsuDifficultyAttributes(aim_difficulty=value):
            return value
        case _:
            return None


def pp(beatmap: Beatmap, attributes: DifficultyAttributes) -> float:
    play: PerformanceAttributes = Performance(
        max_combo=500, statistics={HitResult.GREAT: 400, HitResult.MISS: 2}
    ).calculate(beatmap, attributes)
    return play.total


def aim_graph(beatmap: Beatmap) -> list[float]:
    strains: Strains = Difficulty(mods="HD").strains(beatmap)
    if isinstance(strains, OsuStrains):
        return strains.aim
    return []
