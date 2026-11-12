"""A parsed beatmap and its difficulty settings."""

from __future__ import annotations

import ctypes
import os
from dataclasses import dataclass

from pppp._ctypes import _native
from pppp._ctypes._native import struct_PpppNativeBeatmap as PpppNativeBeatmap
from pppp.errors import ParseError
from pppp.native import check_status, read_string


@dataclass(frozen=True)
class BeatmapDifficulty:
    """The difficulty settings of a parsed beatmap."""

    drain_rate: float
    circle_size: float
    overall_difficulty: float
    approach_rate: float
    slider_multiplier: float
    slider_tick_rate: float


class Beatmap:
    """A parsed beatmap."""

    __slots__: tuple[str, ...] = ("_native",)

    def __init__(self, native: PpppNativeBeatmap) -> None:
        self._native: PpppNativeBeatmap | None = native

    @classmethod
    def from_bytes(cls, data: bytes | bytearray | memoryview) -> Beatmap:
        """Parse a beatmap from the bytes of a .osu file."""
        if isinstance(data, memoryview):
            data = data.tobytes()
        elif isinstance(data, bytearray):
            data = bytes(data)

        # Without type checking, Beatmap.from_bytes("...") reaches this.
        if not isinstance(data, bytes):  # pyright: ignore[reportUnnecessaryIsInstance]
            raise TypeError("data must be bytes-like")  # pyright: ignore[reportUnreachable]

        buffer = ctypes.create_string_buffer(data, max(len(data), 1))
        native = PpppNativeBeatmap()
        status = _native.Pppp_Beatmap_CreateFromBytes(
            ctypes.cast(buffer, ctypes.c_void_p), len(data), ctypes.byref(native)
        )
        check_status(status, ParseError)

        return cls(native)

    @classmethod
    def from_file(cls, path: str | bytes | os.PathLike[str] | os.PathLike[bytes]) -> Beatmap:
        """Parse a beatmap from a .osu file."""
        with open(path, "rb") as file:
            return cls.from_bytes(file.read())

    @classmethod
    def from_text(cls, text: str) -> Beatmap:
        """Parse a beatmap from the text of a .osu file."""
        # Without type checking, Beatmap.from_text(b"...") reaches this.
        if not isinstance(text, str):  # pyright: ignore[reportUnnecessaryIsInstance]
            raise TypeError("text must be a str")  # pyright: ignore[reportUnreachable]

        native = PpppNativeBeatmap()
        status = _native.Pppp_Beatmap_CreateFromText(text.encode("utf-8"), ctypes.byref(native))
        check_status(status, ParseError)

        return cls(native)

    def ensure_open(self) -> PpppNativeBeatmap:
        native = self._native
        if native is None or not bool(native.handle):
            raise ValueError("beatmap is not loaded")

        return native

    @property
    def title(self) -> str:
        native = self.ensure_open()

        return read_string(_native.Pppp_Beatmap_GetTitle, native.handle)

    @property
    def artist(self) -> str:
        native = self.ensure_open()

        return read_string(_native.Pppp_Beatmap_GetArtist, native.handle)

    @property
    def version(self) -> str:
        native = self.ensure_open()

        return read_string(_native.Pppp_Beatmap_GetVersion, native.handle)

    @property
    def beatmap_id(self) -> int:
        return int(self.ensure_open().beatmap_id)

    @property
    def format_version(self) -> int:
        return int(self.ensure_open().format_version)

    @property
    def mode(self) -> int:
        return int(self.ensure_open().ruleset_id)

    @property
    def stack_leniency(self) -> float:
        return float(self.ensure_open().stack_leniency)

    @property
    def difficulty(self) -> BeatmapDifficulty:
        native = self.ensure_open()
        return BeatmapDifficulty(
            drain_rate=float(native.drain_rate),
            circle_size=float(native.circle_size),
            overall_difficulty=float(native.overall_difficulty),
            approach_rate=float(native.approach_rate),
            slider_multiplier=float(native.slider_multiplier),
            slider_tick_rate=float(native.slider_tick_rate),
        )

    def close(self) -> None:
        native = self._native
        if native is None:
            return

        self._native = None
        if bool(native.handle):
            _native.Pppp_Beatmap_Destroy(native.handle)

    def __enter__(self) -> Beatmap:
        return self

    def __exit__(self, *_: object) -> None:
        self.close()

    def __del__(self) -> None:
        try:
            self.close()
        except Exception:
            pass


def require(beatmap: object) -> Beatmap:
    """Check that a Beatmap is open, then return it."""
    if not isinstance(beatmap, Beatmap):
        raise TypeError("expected a Beatmap")
    _ = beatmap.ensure_open()

    return beatmap
