"""A parsed `.osu` file and its lazily read contents."""

from __future__ import annotations

import ctypes
import os
from typing import Any, cast

from . import _native
from .data import (
    BeatmapDifficulty,
    BreakPeriod,
    HitObject,
    Slider,
    TimingPoint,
    break_period,
    difficulty_of,
    hit_object,
    slider,
    timing_point,
    values,
)
from .errors import ParseError
from .native import check


class Beatmap:
    """A loaded beatmap."""

    __slots__ = ("_handle", "_prepared", "_cache")

    def __init__(self, handle: Any) -> None:
        self._handle = handle
        self._prepared = False
        self._cache: dict[str, Any] | None = None

    @classmethod
    def from_bytes(cls, data: bytes | bytearray | memoryview) -> Beatmap:
        """Parse a `Beatmap` by providing the content of a `.osu` file as a slice of bytes."""
        if isinstance(data, memoryview):
            data = data.tobytes()
        elif isinstance(data, bytearray):
            data = bytes(data)
        if not isinstance(data, bytes):
            raise TypeError("data must be bytes-like")
        buffer = ctypes.create_string_buffer(data, max(len(data), 1))
        handle = ctypes.POINTER(_native.pppp_beatmap)()
        status = _native.pppp_beatmap_from_bytes(
            ctypes.cast(buffer, ctypes.c_void_p), len(data), ctypes.byref(handle)
        )
        if status != 0:
            check(status, ParseError)
        return cls(handle)

    @classmethod
    def from_file(cls, path: str | bytes | os.PathLike[str] | os.PathLike[bytes]) -> Beatmap:
        """Parse a `Beatmap` by providing a path to a `.osu` file."""
        with open(path, "rb") as file:
            return cls.from_bytes(file.read())

    def __del__(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle is not None and bool(handle):
            _native.pppp_beatmap_free(handle)

    def _ensure_open(self) -> Any:
        if not bool(self._handle):
            raise AttributeError("beatmap is not loaded")
        return self._handle

    def _prepare(self) -> None:
        handle = self._ensure_open()
        if self._prepared:
            return
        check(_native.pppp_beatmap_prepare_data(handle))
        self._prepared = True

    def _data(self) -> dict[str, Any]:
        self._prepare()
        if self._cache is None:
            handle = self._ensure_open()
            difficulty = _native.pppp_beatmap_difficulty()
            check(_native.pppp_beatmap_get_difficulty(handle, ctypes.byref(difficulty)))
            objects, object_count = ctypes.POINTER(_native.pppp_hit_object)(), ctypes.c_size_t()
            check(
                _native.pppp_beatmap_hit_objects(
                    handle, ctypes.byref(objects), ctypes.byref(object_count)
                )
            )
            sliders, slider_count = ctypes.POINTER(_native.pppp_slider)(), ctypes.c_size_t()
            check(
                _native.pppp_beatmap_sliders(
                    handle, ctypes.byref(sliders), ctypes.byref(slider_count)
                )
            )
            points, point_count = ctypes.POINTER(_native.pppp_timing_point)(), ctypes.c_size_t()
            check(
                _native.pppp_beatmap_timing_points(
                    handle, ctypes.byref(points), ctypes.byref(point_count)
                )
            )
            breaks, break_count = ctypes.POINTER(_native.pppp_break_period)(), ctypes.c_size_t()
            check(
                _native.pppp_beatmap_breaks(handle, ctypes.byref(breaks), ctypes.byref(break_count))
            )
            self._cache = {
                "difficulty": difficulty_of(difficulty),
                "hit_objects": [
                    hit_object(item) for item in values(objects, object_count.value)
                ],
                "sliders": [slider(item) for item in values(sliders, slider_count.value)],
                "timing_points": [
                    timing_point(item) for item in values(points, point_count.value)
                ],
                "breaks": [break_period(item) for item in values(breaks, break_count.value)],
            }
        return self._cache

    @property
    def format_version(self) -> int:
        """The `.osu` format version."""
        return int(_native.pppp_beatmap_format_version(self._ensure_open()))

    @property
    def mode(self) -> int:
        """The beatmap's ruleset id."""
        return int(_native.pppp_beatmap_mode(self._ensure_open()))

    @property
    def stack_leniency(self) -> float:
        """The beatmap's stacking leniency."""
        return float(_native.pppp_beatmap_stack_leniency(self._ensure_open()))

    @property
    def difficulty(self) -> BeatmapDifficulty:
        """The top-level difficulty settings."""
        return cast(BeatmapDifficulty, self._data()["difficulty"])

    @property
    def hit_objects(self) -> list[HitObject]:
        """The map's hit objects."""
        return cast(list[HitObject], self._data()["hit_objects"])

    @property
    def sliders(self) -> list[Slider]:
        """The map's sliders."""
        return cast(list[Slider], self._data()["sliders"])

    @property
    def timing_points(self) -> list[TimingPoint]:
        """The map's timing points."""
        return cast(list[TimingPoint], self._data()["timing_points"])

    @property
    def breaks(self) -> list[BreakPeriod]:
        """The map's break periods."""
        return cast(list[BreakPeriod], self._data()["breaks"])


def require(beatmap: object) -> Beatmap:
    """Reject anything that is not an open `Beatmap`."""
    if not isinstance(beatmap, Beatmap):
        raise TypeError("expected a Beatmap")
    if not bool(beatmap._handle):
        raise AttributeError("beatmap is not loaded")
    return beatmap
