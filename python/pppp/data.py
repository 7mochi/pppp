"""The frozen data classes that mirror a parsed beatmap, and the converters from the ABI structs."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from . import _native


@dataclass(frozen=True)
class Vector2:
    """A two-dimensional point.

    Attributes:
        x: The x coordinate.
        y: The y coordinate.
    """

    x: float
    y: float


@dataclass(frozen=True)
class BeatmapDifficulty:
    """A representation of all top-level difficulty settings for a beatmap.

    Attributes:
        drain_rate: The drain rate (HP drain).
        circle_size: The circle size.
        overall_difficulty: The overall difficulty.
        approach_rate: The approach rate.
        slider_multiplier: The slider velocity multiplier.
        slider_tick_rate: The number of slider ticks per beat.
    """

    drain_rate: float
    circle_size: float
    overall_difficulty: float
    approach_rate: float
    slider_multiplier: float
    slider_tick_rate: float


@dataclass(frozen=True)
class SliderEvent:
    """Describes a point in time on a slider given special meaning.

    Attributes:
        type: The event type.
        time: The time of the event relative to the slider start.
        span_index: The index of the slider span the event belongs to.
        span_start_time: The start time of that span.
        path_progress: The progress along the slider path at the event.
        position: The position of the event on the playfield.
    """

    type: int
    time: float
    span_index: int
    span_start_time: float
    path_progress: float
    position: Vector2


@dataclass(frozen=True)
class HitObject:
    """A HitObject describes an object in a Beatmap.

    Attributes:
        position: The position of the object on the playfield.
        type: The object type bitfield.
        hitsound: The hitsound bitfield.
        start_time: The start time in milliseconds.
        end_time: The end time in milliseconds.
        new_combo: Whether the object starts a new combo.
        combo_offset: The combo colour offset.
        slider: The index of the object's slider, or -1 if it is not a slider.
    """

    position: Vector2
    type: int
    hitsound: int
    start_time: float
    end_time: float
    new_combo: bool
    combo_offset: int
    slider: int


@dataclass(frozen=True)
class Slider:
    """A slider and the arrays that describe it. Every pointer belongs to the beatmap and stays
    valid until `pppp_beatmap_free`.

    Attributes:
        slides: The number of slides.
        expected_length: The expected length of the slider.
        node_sounds: The hitsound of each node.
        control_points: The control points that define the path.
        path: The decimated points of the path.
        cumulative_lengths: The cumulative length at each path point.
        undecimated_path: The undecimated points of the path.
        undecimated_cumulative_lengths: The cumulative length at each undecimated path point.
        events: The legacy events on the slider.
        catch_events: The events used by the osu!catch conversion.
    """

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


@dataclass(frozen=True)
class TimingPoint:
    """A timing point from the beatmap.

    Attributes:
        time: The time of the timing point in milliseconds.
        beat_length: The beat length, or the slider velocity for inherited points.
        meter: The number of beats per measure.
        uninherited: Whether this is an uninherited timing point.
        effects: The effects bitfield.
    """

    time: float
    beat_length: float
    meter: int
    uninherited: bool
    effects: int


@dataclass(frozen=True)
class BreakPeriod:
    """A break in the beatmap.

    Attributes:
        start_time: The start time of the break in milliseconds.
        end_time: The end time of the break in milliseconds.
    """

    start_time: float
    end_time: float


def values(pointer: Any, count: int) -> list[Any]:
    """Read `count` entries of an array that comes from the beatmap, treating null as empty."""
    if not pointer or not count:
        return []
    return [pointer[index] for index in range(int(count))]


def doubles(pointer: Any, count: int) -> list[float]:
    """Read an array of doubles that comes from the beatmap."""
    return [float(value) for value in values(pointer, count)]


def vector(native: Any) -> Vector2:
    """Convert a `pppp_vector2`."""
    return Vector2(x=float(native.x), y=float(native.y))


def slider_event(native: Any) -> SliderEvent:
    """Convert a `pppp_slider_event_descriptor`."""
    return SliderEvent(
        type=int(native.type),
        time=float(native.time),
        span_index=int(native.span_index),
        span_start_time=float(native.span_start_time),
        path_progress=float(native.path_progress),
        position=vector(native.position),
    )


def difficulty_of(native: Any) -> BeatmapDifficulty:
    """Convert a `pppp_beatmap_difficulty`."""
    return BeatmapDifficulty(
        drain_rate=float(native.drain_rate),
        circle_size=float(native.circle_size),
        overall_difficulty=float(native.overall_difficulty),
        approach_rate=float(native.approach_rate),
        slider_multiplier=float(native.slider_multiplier),
        slider_tick_rate=float(native.slider_tick_rate),
    )


def hit_object(native: Any) -> HitObject:
    """Convert a `pppp_hit_object`."""
    return HitObject(
        position=vector(native.position),
        type=int(native.type),
        hitsound=int(native.hitsound),
        start_time=float(native.start_time),
        end_time=float(native.end_time),
        new_combo=bool(native.new_combo),
        combo_offset=int(native.combo_offset),
        slider=int(native.slider),
    )


def slider(native: Any) -> Slider:
    """Convert a `pppp_slider`."""
    return Slider(
        slides=int(native.slides),
        expected_length=float(native.expected_length),
        node_sounds=[int(item) for item in values(native.node_sounds, native.node_sound_count)],
        control_points=[
            vector(item) for item in values(native.control_points, native.control_point_count)
        ],
        path=[vector(item) for item in values(native.path, native.path_count)],
        cumulative_lengths=doubles(native.cumulative_lengths, native.cumulative_length_count),
        undecimated_path=[
            vector(item) for item in values(native.undecimated_path, native.undecimated_path_count)
        ],
        undecimated_cumulative_lengths=doubles(
            native.undecimated_cumulative_lengths, native.undecimated_cumulative_length_count
        ),
        events=[slider_event(item) for item in values(native.events, native.event_count)],
        catch_events=[
            slider_event(item) for item in values(native.catch_events, native.catch_event_count)
        ],
    )


def timing_point(native: Any) -> TimingPoint:
    """Convert a `pppp_timing_point`."""
    return TimingPoint(
        time=float(native.time),
        beat_length=float(native.beat_length),
        meter=int(native.meter),
        uninherited=bool(native.uninherited),
        effects=int(native.effects),
    )


def break_period(native: Any) -> BreakPeriod:
    """Convert a `pppp_break_period`."""
    return BreakPeriod(start_time=float(native.start_time), end_time=float(native.end_time))
