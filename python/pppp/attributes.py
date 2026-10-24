"""The frozen attribute sets a calculation returns, and their converters from the ABI structs."""

from __future__ import annotations

from dataclasses import dataclass, fields
from typing import Any, ClassVar, TypeAlias

from . import _native
from .ruleset import Ruleset


def _require_numbers(attributes: Any) -> None:
    """Reject a dataclass field the caller set to something the ABI cannot take."""
    for field in fields(attributes):
        value = getattr(attributes, field.name)
        if not isinstance(value, (int, float)):
            raise TypeError(f"{field.name} must be numbers")


@dataclass(frozen=True)
class _DifficultyAttributesBase:
    """The difficulty fields every ruleset shares."""

    star_rating: float
    max_combo: int


@dataclass(frozen=True)
class _PerformanceAttributesBase:
    """The performance fields every ruleset shares."""

    total: float


_OSU_DIFFICULTY_FIELDS = (
    "aim_difficulty",
    "speed_difficulty",
    "reading_difficulty",
    "flashlight_difficulty",
    "slider_factor",
    "aim_difficult_strain_count",
    "speed_difficult_strain_count",
    "reading_difficult_note_count",
    "aim_difficult_slider_count",
    "aim_top_weighted_slider_factor",
    "speed_top_weighted_slider_factor",
    "speed_note_count",
    "hit_circle_count",
    "slider_count",
    "large_tick_count",
    "spinner_count",
    "nested_score_per_object",
    "legacy_score_base_multiplier",
    "maximum_legacy_combo_score",
)

_TAIKO_DIFFICULTY_FIELDS = (
    "mechanical_difficulty",
    "rhythm_difficulty",
    "reading_difficulty",
    "colour_difficulty",
    "stamina_difficulty",
    "mono_stamina_factor",
    "consistency_factor",
    "stamina_top_strains",
)


@dataclass(frozen=True)
class OsuDifficultyAttributes(_DifficultyAttributesBase):
    """Difficulty attributes for osu!standard mode.

    Attributes:
        star_rating: The combined star rating of all skills.
        max_combo: The maximum achievable combo.
        aim_difficulty: The difficulty corresponding to the aim skill.
        speed_difficulty: The difficulty corresponding to the speed skill.
        reading_difficulty: The difficulty corresponding to the reading skill.
        flashlight_difficulty: The difficulty corresponding to the flashlight skill.
        slider_factor: How much of the aim difficulty is contributed by hitcircles or sliders.
        aim_difficult_strain_count: The number of difficult aim strains.
        speed_difficult_strain_count: The number of difficult speed strains.
        reading_difficult_note_count: The number of difficult reading notes.
        aim_difficult_slider_count: The number of sliders weighted by difficulty.
        aim_top_weighted_slider_factor: How much of the aim difficult strain count is contributed by hitcircles or sliders.
        speed_top_weighted_slider_factor: How much of the speed difficult strain count is contributed by hitcircles or sliders.
        speed_note_count: The number of clickable objects weighted by difficulty.
        hit_circle_count: The number of hitcircles in the beatmap.
        slider_count: The number of sliders in the beatmap.
        large_tick_count: The number of large ticks in the beatmap.
        spinner_count: The number of spinners in the beatmap.
        nested_score_per_object: The nested score per object.
        legacy_score_base_multiplier: The base multiplier for legacy scoring.
        maximum_legacy_combo_score: The maximum legacy combo score.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.OSU
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

    @classmethod
    def from_native(cls, native: _native.pppp_difficulty_attributes) -> OsuDifficultyAttributes:
        """Read the osu!standard attributes."""
        return cls(
            star_rating=native.star_rating,
            max_combo=native.max_combo,
            aim_difficulty=native.osu.aim_difficulty,
            speed_difficulty=native.osu.speed_difficulty,
            reading_difficulty=native.osu.reading_difficulty,
            flashlight_difficulty=native.osu.flashlight_difficulty,
            slider_factor=native.osu.slider_factor,
            aim_difficult_strain_count=native.osu.aim_difficult_strain_count,
            speed_difficult_strain_count=native.osu.speed_difficult_strain_count,
            reading_difficult_note_count=native.osu.reading_difficult_note_count,
            aim_difficult_slider_count=native.osu.aim_difficult_slider_count,
            aim_top_weighted_slider_factor=native.osu.aim_top_weighted_slider_factor,
            speed_top_weighted_slider_factor=native.osu.speed_top_weighted_slider_factor,
            speed_note_count=native.osu.speed_note_count,
            hit_circle_count=native.osu.hit_circle_count,
            slider_count=native.osu.slider_count,
            large_tick_count=native.osu.large_tick_count,
            spinner_count=native.osu.spinner_count,
            nested_score_per_object=native.osu.nested_score_per_object,
            legacy_score_base_multiplier=native.osu.legacy_score_base_multiplier,
            maximum_legacy_combo_score=native.osu.maximum_legacy_combo_score,
        )

    def to_native(self) -> _native.pppp_difficulty_attributes:
        """Write these attributes into a `pppp_difficulty_attributes`."""
        _require_numbers(self)
        native = _native.pppp_difficulty_attributes()
        native.ruleset = int(self.ruleset)
        native.star_rating = self.star_rating
        native.max_combo = self.max_combo
        for name in _OSU_DIFFICULTY_FIELDS:
            setattr(native.osu, name, getattr(self, name))
        return native


@dataclass(frozen=True)
class TaikoDifficultyAttributes(_DifficultyAttributesBase):
    """Difficulty attributes for osu!taiko mode.

    Attributes:
        star_rating: The combined star rating of all skills.
        max_combo: The maximum achievable combo.
        mechanical_difficulty: The difficulty of the mechanical skills, combining colour and stamina.
        rhythm_difficulty: The difficulty corresponding to the rhythm skill.
        reading_difficulty: The difficulty corresponding to the reading skill.
        colour_difficulty: The difficulty corresponding to the colour skill.
        stamina_difficulty: The difficulty corresponding to the stamina skill.
        mono_stamina_factor: The ratio of stamina difficulty from single-colour streams to total stamina difficulty.
        consistency_factor: The factor corresponding to the consistency of a map.
        stamina_top_strains: The top stamina strains.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    mechanical_difficulty: float
    rhythm_difficulty: float
    reading_difficulty: float
    colour_difficulty: float
    stamina_difficulty: float
    mono_stamina_factor: float
    consistency_factor: float
    stamina_top_strains: float

    @classmethod
    def from_native(cls, native: _native.pppp_difficulty_attributes) -> TaikoDifficultyAttributes:
        """Read the osu!taiko attributes."""
        return cls(
            star_rating=native.star_rating,
            max_combo=native.max_combo,
            mechanical_difficulty=native.taiko.mechanical_difficulty,
            rhythm_difficulty=native.taiko.rhythm_difficulty,
            reading_difficulty=native.taiko.reading_difficulty,
            colour_difficulty=native.taiko.colour_difficulty,
            stamina_difficulty=native.taiko.stamina_difficulty,
            mono_stamina_factor=native.taiko.mono_stamina_factor,
            consistency_factor=native.taiko.consistency_factor,
            stamina_top_strains=native.taiko.stamina_top_strains,
        )

    def to_native(self) -> _native.pppp_difficulty_attributes:
        """Write these attributes into a `pppp_difficulty_attributes`."""
        _require_numbers(self)
        native = _native.pppp_difficulty_attributes()
        native.ruleset = int(self.ruleset)
        native.star_rating = self.star_rating
        native.max_combo = self.max_combo
        for name in _TAIKO_DIFFICULTY_FIELDS:
            setattr(native.taiko, name, getattr(self, name))
        return native


@dataclass(frozen=True)
class CatchDifficultyAttributes(_DifficultyAttributesBase):
    """Difficulty attributes for osu!catch mode. This class has no additional attributes beyond the base class.

    Attributes:
        star_rating: The combined star rating of all skills.
        max_combo: The maximum achievable combo.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.CATCH

    @classmethod
    def from_native(cls, native: _native.pppp_difficulty_attributes) -> CatchDifficultyAttributes:
        """Read the osu!catch attributes."""
        return cls(star_rating=native.star_rating, max_combo=native.max_combo)

    def to_native(self) -> _native.pppp_difficulty_attributes:
        """Write these attributes into a `pppp_difficulty_attributes`."""
        _require_numbers(self)
        native = _native.pppp_difficulty_attributes()
        native.ruleset = int(self.ruleset)
        native.star_rating = self.star_rating
        native.max_combo = self.max_combo
        return native


@dataclass(frozen=True)
class ManiaDifficultyAttributes(_DifficultyAttributesBase):
    """Difficulty attributes for osu!mania mode. This class has no additional attributes beyond the base class.

    Attributes:
        star_rating: The combined star rating of all skills.
        max_combo: The maximum achievable combo.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.MANIA

    @classmethod
    def from_native(cls, native: _native.pppp_difficulty_attributes) -> ManiaDifficultyAttributes:
        """Read the osu!mania attributes."""
        return cls(star_rating=native.star_rating, max_combo=native.max_combo)

    def to_native(self) -> _native.pppp_difficulty_attributes:
        """Write these attributes into a `pppp_difficulty_attributes`."""
        _require_numbers(self)
        native = _native.pppp_difficulty_attributes()
        native.ruleset = int(self.ruleset)
        native.star_rating = self.star_rating
        native.max_combo = self.max_combo
        return native


@dataclass(frozen=True)
class TimedDifficultyAttributes:
    """Wraps a `pppp_difficulty_attributes` and adds a time value for which the attribute is valid.
    Output by `pppp_calculate_timed_difficulty`.

    Attributes:
        time: The non-clock-adjusted time value at which the attributes take effect.
        attributes: The attributes.
    """

    time: float
    attributes: DifficultyAttributes


@dataclass(frozen=True)
class OsuPerformanceAttributes(_PerformanceAttributesBase):
    """Performance attributes for osu!standard mode.

    Attributes:
        total: The final calculated performance points.
        aim: The aim portion of the final performance points.
        speed: The speed portion of the final performance points.
        accuracy: The accuracy portion of the final performance points.
        flashlight: The flashlight portion of the final performance points.
        reading: The reading portion of the final performance points.
        effective_miss_count: Misses including an approximated amount of slider breaks.
        combo_based_estimated_miss_count: Estimated miss count based on the player's combo.
        score_based_estimated_miss_count: Estimated miss count based on the player's score, if available.
        aim_estimated_slider_breaks: Estimated number of slider breaks that affected the aim portion.
        speed_estimated_slider_breaks: Estimated number of slider breaks that affected the speed portion.
        speed_deviation: Approximated unstable rate, if available.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.OSU
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

    @classmethod
    def from_native(cls, native: _native.pppp_performance_attributes) -> OsuPerformanceAttributes:
        """Read the osu!standard attributes."""
        osu = native.osu
        return cls(
            total=native.total,
            aim=osu.aim,
            speed=osu.speed,
            accuracy=osu.accuracy,
            flashlight=osu.flashlight,
            reading=osu.reading,
            effective_miss_count=osu.effective_miss_count,
            combo_based_estimated_miss_count=osu.combo_based_estimated_miss_count,
            score_based_estimated_miss_count=(
                osu.score_based_estimated_miss_count
                if osu.has_score_based_estimated_miss_count
                else None
            ),
            aim_estimated_slider_breaks=osu.aim_estimated_slider_breaks,
            speed_estimated_slider_breaks=osu.speed_estimated_slider_breaks,
            speed_deviation=(osu.speed_deviation if osu.has_speed_deviation else None),
        )


@dataclass(frozen=True)
class TaikoPerformanceAttributes(_PerformanceAttributesBase):
    """Performance attributes for osu!taiko mode.

    Attributes:
        total: The final calculated performance points.
        difficulty: The difficulty portion of the final performance points.
        accuracy: The accuracy portion of the final performance points.
        estimated_unstable_rate: Approximated unstable rate, if available.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.TAIKO
    difficulty: float
    accuracy: float
    estimated_unstable_rate: float | None

    @classmethod
    def from_native(cls, native: _native.pppp_performance_attributes) -> TaikoPerformanceAttributes:
        """Read the osu!taiko attributes."""
        taiko = native.taiko
        return cls(
            total=native.total,
            difficulty=taiko.difficulty,
            accuracy=taiko.accuracy,
            estimated_unstable_rate=(
                taiko.estimated_unstable_rate if taiko.has_estimated_unstable_rate else None
            ),
        )


@dataclass(frozen=True)
class CatchPerformanceAttributes(_PerformanceAttributesBase):
    """Performance attributes for osu!catch mode. This class has no additional attributes beyond the base class.

    Attributes:
        total: The final calculated performance points.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.CATCH

    @classmethod
    def from_native(
        cls, native: _native.pppp_performance_attributes
    ) -> CatchPerformanceAttributes:
        """Read the osu!catch attributes."""
        return cls(total=native.total)


@dataclass(frozen=True)
class ManiaPerformanceAttributes(_PerformanceAttributesBase):
    """Performance attributes for osu!mania mode.

    Attributes:
        total: The final calculated performance points.
        difficulty: The difficulty portion of the final performance points.
    """

    ruleset: ClassVar[Ruleset] = Ruleset.MANIA
    difficulty: float

    @classmethod
    def from_native(cls, native: _native.pppp_performance_attributes) -> ManiaPerformanceAttributes:
        """Read the osu!mania attributes."""
        return cls(total=native.total, difficulty=native.mania.difficulty)


DifficultyAttributes: TypeAlias = (
    OsuDifficultyAttributes
    | TaikoDifficultyAttributes
    | CatchDifficultyAttributes
    | ManiaDifficultyAttributes
)

PerformanceAttributes: TypeAlias = (
    OsuPerformanceAttributes
    | TaikoPerformanceAttributes
    | CatchPerformanceAttributes
    | ManiaPerformanceAttributes
)
