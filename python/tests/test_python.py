import copy
import gc
import os
import pickle
import weakref
from pathlib import Path

import pytest

import pppp
from pppp._model import Vector2

RESOURCES = Path(__file__).resolve().parents[2] / "tests" / "resources"


def beatmap_of(name: str) -> pppp.Beatmap:
    return pppp.from_file(RESOURCES / "osu" / name)


def test_version_is_a_version():
    assert isinstance(pppp.version, str)
    assert pppp.version.count(".") == 2


def test_fields_read_back():
    point = Vector2(1.5, -2.5)
    assert point.x == 1.5
    assert point.y == -2.5
    assert Vector2(x=1.5, y=-2.5) == point


def test_records_are_immutable():
    point = Vector2(1.0, 2.0)
    with pytest.raises(AttributeError):
        point.x = 3.0


def test_missing_field_is_an_error():
    with pytest.raises(TypeError):
        Vector2(1.0)


def test_extra_field_is_an_error():
    with pytest.raises(TypeError):
        Vector2(1.0, 2.0, 3.0)


def test_repr():
    assert repr(Vector2(1.0, 2.0)) == "Vector2(x=1.0, y=2.0)"


def test_pickle_round_trip():
    point = Vector2(1.5, -2.5)
    assert pickle.loads(pickle.dumps(point)) == point


def test_deepcopy():
    point = Vector2(1.5, -2.5)
    assert copy.deepcopy(point) == point


def test_the_collector_sees_through_a_record():
    class Probe:
        pass

    probe = Probe()
    reference = weakref.ref(probe)
    holder = [probe]
    probe.record = Vector2(holder, holder)
    del probe, holder
    gc.collect()
    assert reference() is None


def test_from_file_reads_the_model():
    beatmap = beatmap_of("diffcalc-test.osu")
    assert beatmap.format_version == 14
    assert beatmap.mode == 0
    assert beatmap.stack_leniency == pytest.approx(0.3)
    assert beatmap.difficulty.drain_rate == 5.0
    assert beatmap.difficulty.circle_size == 4.0
    assert beatmap.difficulty.overall_difficulty == 7.0
    assert beatmap.difficulty.approach_rate == pytest.approx(8.3)
    assert beatmap.difficulty.slider_multiplier == 1.6
    assert beatmap.difficulty.slider_tick_rate == 1.0
    assert len(beatmap.hit_objects) == 124
    assert len(beatmap.sliders) == 33
    assert len(beatmap.timing_points) == 3
    assert len(beatmap.breaks) == 0


def test_from_file_takes_a_path_a_string_or_bytes():
    path = RESOURCES / "osu" / "diffcalc-test.osu"
    assert len(pppp.from_file(path).hit_objects) == 124
    assert len(pppp.from_file(str(path)).hit_objects) == 124
    assert len(pppp.from_file(os.fsencode(path)).hit_objects) == 124


def test_hit_objects_point_at_their_sliders():
    beatmap = beatmap_of("diffcalc-test.osu")
    object_sliders = [o for o in beatmap.hit_objects if o.slider >= 0]
    assert len(object_sliders) == len(beatmap.sliders)
    assert all(0 <= o.slider < len(beatmap.sliders) for o in object_sliders)


def test_slider_geometry_is_exposed():
    slider = beatmap_of("diffcalc-test.osu").sliders[0]
    assert slider.slides == 1
    assert slider.expected_length == 160.0
    assert len(slider.control_points) == 1
    assert len(slider.events) == 3
    assert len(slider.path) == len(slider.cumulative_lengths)


def test_a_spinner_is_centred():
    last = beatmap_of("diffcalc-test.osu").hit_objects[-1]
    assert last.type & 8
    assert (last.position.x, last.position.y) == (256.0, 192.0)
    assert last.start_time == 102125.0
    assert last.end_time == 103000.0


def test_a_missing_file_is_an_error():
    with pytest.raises(ValueError):
        pppp.from_file(RESOURCES / "osu" / "does-not-exist.osu")


def test_a_null_byte_in_the_path_is_an_error():
    with pytest.raises(ValueError):
        pppp.from_file(b"a\0b")


@pytest.mark.parametrize(
    ("name", "stars", "combo"),
    [
        ("osu/2785319.osu", 6.004027372552197, 909),
        ("taiko/1028484.osu", 2.9144215641617333, 289),
        ("fruits/2118524.osu", 3.2340182503279706, 730),
        ("mania/1638954.osu", 3.358304846842773, 956),
    ],
)
def test_difficulty_matches_the_pinned_stars(name: str, stars: float, combo: int):
    attributes = pppp.Difficulty(pppp.from_file(RESOURCES / name)).calculate()
    assert attributes.star_rating == pytest.approx(stars)
    assert attributes.max_combo == combo


def test_mods_change_the_calculation():
    beatmap = pppp.from_file(RESOURCES / "fruits/2118524.osu")
    assert pppp.Difficulty(beatmap).mods("HR").calculate().star_rating == pytest.approx(
        4.308291009137178
    )


def test_the_ruleset_can_be_forced():
    beatmap = pppp.from_file(RESOURCES / "osu/2785319.osu")
    attributes = pppp.Difficulty(beatmap, ruleset=pppp.Ruleset.TAIKO).calculate()
    assert attributes.ruleset == pppp.Ruleset.TAIKO
    assert attributes.taiko.star_rating == pytest.approx(4.752572620626138)
    assert attributes.taiko.mechanical_difficulty > 0.0


def test_a_taiko_map_fills_the_taiko_attributes():
    attributes = pppp.Difficulty(pppp.from_file(RESOURCES / "taiko/1028484.osu")).calculate()
    assert attributes.ruleset == pppp.Ruleset.TAIKO
    assert attributes.osu.star_rating == 0.0
    assert attributes.taiko.mechanical_difficulty > 0.0


def test_an_unknown_mod_is_an_error():
    beatmap = pppp.from_file(RESOURCES / "osu/2785319.osu")
    with pytest.raises(ValueError):
        pppp.Difficulty(beatmap).mods("XX").calculate()


def test_a_beatmap_is_required():
    with pytest.raises(TypeError):
        pppp.Difficulty("not a beatmap").calculate()  # type: ignore[arg-type]


# The hit-result indices are the library's: `GREAT = 5` and `SLIDER_TAIL_HIT = 16`.
def score_of(counts: dict[int, int], combo: int, accuracy: float = 1.0) -> pppp.ScoreInfo:
    statistics = [0] * 18
    for index, count in counts.items():
        statistics[index] = count
    return pppp.ScoreInfo(
        statistics=statistics,
        maximum_statistics=[0] * 18,
        max_combo=combo,
        accuracy=accuracy,
        legacy_total_score=None,
    )


def test_performance_of_osu_matches_the_pinned_pp():
    beatmap = pppp.from_file(RESOURCES / "osu" / "2785319.osu")
    difficulty = pppp.Difficulty(beatmap).calculate()
    # The score carries the slider tails, as the library's own test does.
    play = pppp.Performance(
        beatmap, state=score_of({5: 601, 16: difficulty.osu.slider_count}, 909)
    ).calculate()
    assert play.total == pytest.approx(316.5901855625614)
    assert play.osu.aim == pytest.approx(148.75278891878943)
    assert play.osu.speed == pytest.approx(61.34653468094172)
    assert play.osu.accuracy == pytest.approx(98.99847982709288)
    assert play.osu.reading == pytest.approx(2.2291238201795176)
    assert play.osu.score_based_estimated_miss_count is None


def test_performance_of_taiko_matches_the_pinned_pp():
    beatmap = pppp.from_file(RESOURCES / "taiko" / "1028484.osu")
    play = pppp.Performance(beatmap, state=score_of({5: 289}, 289)).calculate()
    assert play.ruleset == pppp.Ruleset.TAIKO
    assert play.total == pytest.approx(130.26636361095524)
    assert play.taiko.difficulty == pytest.approx(33.48488833057447)
    assert play.taiko.accuracy == pytest.approx(96.78147528038076)
    assert play.taiko.estimated_unstable_rate == pytest.approx(146.3238357972284)


def test_performance_of_catch_matches_the_pinned_pp():
    beatmap = pppp.from_file(RESOURCES / "fruits" / "2118524.osu")
    difficulty = pppp.Difficulty(beatmap).calculate()
    play = pppp.Performance(
        beatmap, state=score_of({5: 728, 10: 2, 8: 263}, difficulty.max_combo)
    ).calculate()
    assert play.ruleset == pppp.Ruleset.CATCH
    assert play.total == pytest.approx(112.72215339177879)


def test_performance_of_mania_matches_the_pinned_pp():
    beatmap = pppp.from_file(RESOURCES / "mania" / "1638954.osu")
    difficulty = pppp.Difficulty(beatmap).calculate()
    play = pppp.Performance(
        beatmap, state=score_of({6: 715}, difficulty.max_combo)
    ).calculate()
    assert play.ruleset == pppp.Ruleset.MANIA
    assert play.total == pytest.approx(108.92297471705167)


def test_performance_takes_the_combo_and_misses_it_is_given():
    beatmap = pppp.from_file(RESOURCES / "osu" / "2785319.osu")
    play = pppp.Performance(beatmap, combo=909, accuracy=1.0, misses=0).calculate()
    assert play.total > 0.0


def test_performance_from_attributes_matches_the_map():
    beatmap = pppp.from_file(RESOURCES / "osu" / "2785319.osu")
    difficulty = pppp.Difficulty(beatmap).calculate()
    score = score_of({5: 601, 16: difficulty.osu.slider_count}, 909)
    from_map = pppp.Performance(beatmap, state=score).calculate()
    from_attributes = pppp.Performance(beatmap, state=score, attributes=difficulty).calculate()
    assert from_attributes.total == from_map.total
    assert from_attributes.osu.aim == from_map.osu.aim
    assert from_attributes == from_map
