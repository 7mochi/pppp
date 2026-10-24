"""The strain series a difficulty calculation returns, per ruleset."""

from __future__ import annotations

from dataclasses import dataclass
from typing import ClassVar, TypeAlias

from .ruleset import Ruleset


@dataclass(frozen=True)
class _StrainsBase:
    """The fields every ruleset's strain series shares."""

    start_time: float
    section_length: float


@dataclass(frozen=True)
class OsuStrains(_StrainsBase):
    """The series of an osu! strain graph, taken from a `pppp_strains`. An empty series has a null
    pointer and a count of 0.

    Attributes:
        start_time: The time of the first strain section.
        section_length: The length of a strain section.
        aim: The aim strains.
        aim_no_sliders: The aim strains without sliders.
        speed: The speed strains.
        reading: The reading strains.
        flashlight: The flashlight strains.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.OSU
    aim: list[float]
    aim_no_sliders: list[float]
    speed: list[float]
    reading: list[float]
    flashlight: list[float]


@dataclass(frozen=True)
class TaikoStrains(_StrainsBase):
    """The strain series of an osu!taiko map, suitable for plotting difficulty over time.

    Attributes:
        start_time: The time of the first strain section.
        section_length: The length of a strain section.
        colour: The colour strains.
        reading: The reading strains.
        rhythm: The rhythm strains.
        stamina: The stamina strains.
        single_colour_stamina: The single-colour stamina strains.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    colour: list[float]
    reading: list[float]
    rhythm: list[float]
    stamina: list[float]
    single_colour_stamina: list[float]


@dataclass(frozen=True)
class CatchStrains(_StrainsBase):
    """The strain series of an osu!catch map, suitable for plotting difficulty over time.

    Attributes:
        start_time: The time of the first strain section.
        section_length: The length of a strain section.
        movement: The movement strains.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.CATCH
    movement: list[float]


@dataclass(frozen=True)
class ManiaStrains(_StrainsBase):
    """The strain series of an osu!mania map, suitable for plotting difficulty over time.

    Attributes:
        start_time: The time of the first strain section.
        section_length: The length of a strain section.
        strain: The strains.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    strain: list[float]


Strains: TypeAlias = OsuStrains | TaikoStrains | CatchStrains | ManiaStrains
