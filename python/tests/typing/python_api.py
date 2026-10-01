"""The public surface as a typed user would write it; mypy checks this file."""

from pppp import BeatmapDifficulty, BreakPeriod, HitObject, ScoreInfo, Slider, TimingPoint, Vector2


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


def legacy_score(score: ScoreInfo) -> int | None:
    return score.legacy_total_score
