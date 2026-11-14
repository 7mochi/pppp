from __future__ import annotations

import pytest

import pppp
from tests.conftest import RESOURCES


def _osu_score(difficulty):
    return dict(
        max_combo=909,
        accuracy=1.0,
        statistics={
            pppp.HitResult.GREAT: 601,
            pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count,
        },
    )


def test_osu_performance():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    play = pppp.Performance(**_osu_score(difficulty)).calculate(beatmap)
    assert play.total == pytest.approx(316.5901855625614)
    assert play.aim == pytest.approx(148.75278891878943)
    assert play.speed == pytest.approx(61.34653468094172)
    assert play.accuracy == pytest.approx(98.99847982709288)
    assert play.reading == pytest.approx(2.2291238201795176)
    assert play.score_based_estimated_miss_count is None


def test_taiko_performance():
    play = pppp.Performance(
        max_combo=289, accuracy=1.0, statistics={pppp.HitResult.GREAT: 289}
    ).calculate(pppp.Beatmap.from_file(RESOURCES / "taiko" / "1028484.osu"))
    assert play.total == pytest.approx(130.26636361095524)
    assert play.difficulty == pytest.approx(33.48488833057447)
    assert play.accuracy == pytest.approx(96.78147528038076)
    assert play.estimated_unstable_rate == pytest.approx(146.3238357972284)


def test_catch_performance():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "fruits" / "2118524.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    play = pppp.Performance(
        max_combo=difficulty.max_combo,
        accuracy=1.0,
        statistics={
            pppp.HitResult.GREAT: 728,
            pppp.HitResult.LARGE_TICK_HIT: 2,
            pppp.HitResult.SMALL_TICK_HIT: 263,
        },
    ).calculate(beatmap)
    assert play.total == pytest.approx(112.72215339177879)


def test_mania_performance():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "mania" / "1638954.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    play = pppp.Performance(
        max_combo=difficulty.max_combo,
        accuracy=1.0,
        statistics={pppp.HitResult.PERFECT: 715},
    ).calculate(beatmap)
    assert play.total == pytest.approx(108.92297471705167)
    assert play.difficulty == pytest.approx(108.92297471705167)


def test_classic_lazer():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    score = _osu_score(difficulty)
    classic = pppp.Performance(mods="CL", **score).calculate(beatmap)
    twice = pppp.Performance(mods="CL,CL", **score).calculate(beatmap)
    lazer = pppp.Performance(**score).calculate(beatmap)
    assert classic.total == pytest.approx(298.5325815982227)
    assert twice.total == pytest.approx(classic.total)
    assert lazer.total == pytest.approx(316.5901855625614)


def test_classic_option():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    score = _osu_score(difficulty)
    assert pppp.Performance(classic=True, **score).calculate(beatmap).total == pytest.approx(
        298.5325815982227
    )


def test_performance_attributes():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    performance = pppp.Performance(**_osu_score(difficulty))
    assert performance.calculate(beatmap, difficulty) == performance.calculate(beatmap)


def test_statistics_key_outside_abi():
    with pytest.raises(ValueError):
        pppp.Performance(statistics={99: 1}).calculate(
            pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
        )


def test_misses_override_statistics():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    base = pppp.Performance(max_combo=909, accuracy=1.0, misses=0).calculate(beatmap)
    override = pppp.Performance(
        max_combo=909, accuracy=1.0, misses=0, statistics={pppp.HitResult.MISS: 4}
    ).calculate(beatmap)
    assert override == base


@pytest.mark.parametrize("key", [True, 5.7, "5"])
def test_statistics_key_types(key):
    with pytest.raises(ValueError):
        pppp.Performance(statistics={key: 1}).calculate(
            pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
        )
