"""pppp: osu! difficulty and performance points, in native code."""

from __future__ import annotations

from . import library

# The generated `_native` loads `pppp_c` when it is imported, so the directory the glue resolved has
# to be registered first. This is the only place the import order matters.
library.prepare(library.search_directory())

from .attributes import (  # noqa: E402
    CatchDifficultyAttributes,
    CatchPerformanceAttributes,
    DifficultyAttributes,
    ManiaDifficultyAttributes,
    ManiaPerformanceAttributes,
    OsuDifficultyAttributes,
    OsuPerformanceAttributes,
    PerformanceAttributes,
    TaikoDifficultyAttributes,
    TaikoPerformanceAttributes,
    TimedDifficultyAttributes,
)
from .backend import __version__, backend  # noqa: E402
from .beatmap import Beatmap  # noqa: E402
from .data import (  # noqa: E402
    BeatmapDifficulty,
    BreakPeriod,
    HitObject,
    Slider,
    SliderEvent,
    TimingPoint,
    Vector2,
)
from .difficulty import Difficulty  # noqa: E402
from .errors import Error, ModsError, ParseError  # noqa: E402
from .hit_result import HitResult  # noqa: E402
from .performance import Performance  # noqa: E402
from .ruleset import Ruleset  # noqa: E402
from .strains import CatchStrains, ManiaStrains, OsuStrains, Strains, TaikoStrains  # noqa: E402

__all__ = [
    "Beatmap",
    "BeatmapDifficulty",
    "BreakPeriod",
    "CatchDifficultyAttributes",
    "CatchPerformanceAttributes",
    "CatchStrains",
    "Difficulty",
    "DifficultyAttributes",
    "Error",
    "HitObject",
    "HitResult",
    "ManiaDifficultyAttributes",
    "ManiaPerformanceAttributes",
    "ManiaStrains",
    "ModsError",
    "OsuDifficultyAttributes",
    "OsuPerformanceAttributes",
    "OsuStrains",
    "ParseError",
    "Performance",
    "PerformanceAttributes",
    "Ruleset",
    "Slider",
    "SliderEvent",
    "Strains",
    "TaikoDifficultyAttributes",
    "TaikoPerformanceAttributes",
    "TaikoStrains",
    "TimedDifficultyAttributes",
    "TimingPoint",
    "Vector2",
    "__version__",
    "backend",
]
