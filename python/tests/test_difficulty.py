from __future__ import annotations

import ctypes

import pytest

import pppp
from tests.conftest import RESOURCES
from pppp._ctypes import _native


def test_mods_parse():
    with pppp.mods.parse("HD,DT", 0, False) as mods:
        count = _native.PpppInt32()
        assert _native.Pppp_Mods_GetCount(mods, ctypes.byref(count)) == _native.Pppp_Success
        assert count.value == 2


def test_invalid_mods():
    with pytest.raises(pppp.ModsError):
        with pppp.mods.parse("XX", 0, False):
            pass


@pytest.mark.parametrize(
    ("name", "stars", "combo"),
    [
        ("osu/2785319.osu", 6.004027372552197, 909),
        ("taiko/1028484.osu", 2.9144215641617333, 289),
        ("fruits/2118524.osu", 3.2340182503279706, 730),
        ("mania/1638954.osu", 3.358304846842773, 956),
    ],
)
def test_difficulty_values(name, stars, combo):
    beatmap = pppp.Beatmap.from_file(RESOURCES / name)
    attributes = pppp.Difficulty().calculate(beatmap)
    assert attributes.ruleset == pppp.Ruleset(beatmap.mode)
    assert attributes.star_rating == pytest.approx(stars)
    assert attributes.max_combo == combo


def test_mods_change_difficulty():
    attributes = pppp.Difficulty(mods="HR").calculate(
        pppp.Beatmap.from_file(RESOURCES / "fruits" / "2118524.osu")
    )
    assert attributes.star_rating == pytest.approx(4.308291009137178)


def test_stable_mod_bits():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "fruits" / "2118524.osu")
    assert pppp.Difficulty(mods=16).calculate(beatmap) == pppp.Difficulty(mods="HR").calculate(
        beatmap
    )


def test_forced_ruleset():
    attributes = pppp.Difficulty(ruleset=pppp.Ruleset.TAIKO).calculate(
        pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    )
    assert attributes.ruleset is pppp.Ruleset.TAIKO
    assert attributes.star_rating == pytest.approx(4.752572620626138)


def test_clock_rate():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "diffcalc-test.osu")
    normal = pppp.Difficulty(mods="DT").calculate(beatmap)
    faster = pppp.Difficulty(mods="DT", clock_rate=2.0).calculate(beatmap)
    assert normal.star_rating == pytest.approx(9.467769487798352)
    assert faster.star_rating == pytest.approx(12.120899066435245)


def test_unknown_mods():
    with pytest.raises(pppp.ModsError):
        pppp.Difficulty(mods="XX").calculate(
            pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
        )


def test_unknown_ruleset():
    with pytest.raises(pppp.Error):
        pppp.Difficulty(ruleset=9).calculate(
            pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
        )


def test_beatmap_required():
    with pytest.raises(TypeError):
        pppp.Difficulty().calculate("not a beatmap")


def test_closed_beatmap_calculation():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    beatmap.close()
    with pytest.raises(ValueError):
        pppp.Difficulty().calculate(beatmap)


def test_timed_values():
    timed = pppp.Difficulty().calculate_timed(
        pppp.Beatmap.from_file(RESOURCES / "osu" / "diffcalc-test.osu")
    )
    assert len(timed) == 124
    assert timed[62].time == 18000
    assert timed[62].attributes.star_rating == pytest.approx(5.727266936277771)
    assert timed[62].attributes.max_combo == 63
    assert timed[-1].time == 103000
    assert timed[-1].attributes.max_combo == 239


def test_timed_empty():
    text = "osu file format v14\n[General]\nMode: 0\n"
    assert pppp.Difficulty().calculate_timed(pppp.Beatmap.from_text(text)) == []


@pytest.mark.parametrize("ruleset", list(pppp.Ruleset))
def test_timed_rulesets(ruleset):
    timed = pppp.Difficulty(ruleset=ruleset).calculate_timed(
        pppp.Beatmap.from_file(RESOURCES / "osu" / "diffcalc-test.osu")
    )
    assert timed
    assert timed[0].attributes.ruleset is ruleset


def test_osu_strains():
    strains = pppp.Difficulty(mods="HD,FL").calculate_strains(
        pppp.Beatmap.from_file(RESOURCES / "osu" / "diffcalc-test.osu")
    )
    assert isinstance(strains, pppp.OsuStrains)
    assert strains.start_time == 800
    assert strains.section_length == 400
    assert len(strains.flashlight) == 254
    assert strains.flashlight[18] == pytest.approx(41.02690817661656)


@pytest.mark.parametrize(
    ("name", "ruleset", "series"),
    [
        ("taiko/diffcalc-test.osu", pppp.Ruleset.TAIKO, "single_colour_stamina"),
        ("fruits/diffcalc-test.osu", pppp.Ruleset.CATCH, "movement"),
        ("mania/diffcalc-test.osu", pppp.Ruleset.MANIA, "strain"),
    ],
)
def test_strain_records(name, ruleset, series):
    strains = pppp.Difficulty(ruleset=ruleset).calculate_strains(
        pppp.Beatmap.from_file(RESOURCES / name)
    )
    assert strains.ruleset is ruleset
    assert getattr(strains, series)


def test_strains_empty():
    text = "osu file format v14\n[General]\nMode: 0\n"
    strains = pppp.Difficulty().calculate_strains(pppp.Beatmap.from_text(text))
    assert strains.aim == []
    assert strains.speed == []
