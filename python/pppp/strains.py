"""Strain series for each ruleset."""

from __future__ import annotations

from dataclasses import dataclass
from typing import ClassVar, TypeAlias

from pppp.ruleset import Ruleset


@dataclass(frozen=True)
class _StrainsBase:
    start_time: float
    section_length: float


@dataclass(frozen=True)
class OsuStrains(_StrainsBase):
    """Strain series for osu!standard maps."""

    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    aim: list[float]
    aim_no_sliders: list[float]
    speed: list[float]
    reading: list[float]
    flashlight: list[float]


@dataclass(frozen=True)
class TaikoStrains(_StrainsBase):
    """Strain series for osu!taiko maps."""

    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    colour: list[float]
    reading: list[float]
    rhythm: list[float]
    stamina: list[float]
    single_colour_stamina: list[float]


@dataclass(frozen=True)
class CatchStrains(_StrainsBase):
    """Strain series for osu!catch maps."""

    ruleset: ClassVar[Ruleset] = Ruleset.CATCH
    movement: list[float]


@dataclass(frozen=True)
class ManiaStrains(_StrainsBase):
    """Strain series for osu!mania maps."""

    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    strain: list[float]


Strains: TypeAlias = OsuStrains | TaikoStrains | CatchStrains | ManiaStrains
