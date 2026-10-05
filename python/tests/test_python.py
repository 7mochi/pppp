import copy
import dataclasses
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
    return pppp.Beatmap.from_file(RESOURCES / name)


def test_version_is_a_version():
    assert isinstance(pppp.__version__, str)
    assert pppp.__version__.count(".") == 2


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
    beatmap = beatmap_of("osu/diffcalc-test.osu")
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
    assert len(pppp.Beatmap.from_file(path).hit_objects) == 124
    assert len(pppp.Beatmap.from_file(str(path)).hit_objects) == 124
    assert len(pppp.Beatmap.from_file(os.fsencode(path)).hit_objects) == 124


def test_from_bytes_matches_from_file():
    path = RESOURCES / "osu" / "diffcalc-test.osu"
    data = path.read_bytes()
    from_bytes = pppp.Difficulty().calculate(pppp.Beatmap.from_bytes(data))
    from_file = pppp.Difficulty().calculate(pppp.Beatmap.from_file(path))
    assert from_bytes == from_file
    assert pppp.Difficulty().calculate(pppp.Beatmap.from_bytes(bytearray(data))) == from_file
    assert pppp.Difficulty().calculate(pppp.Beatmap.from_bytes(memoryview(data))) == from_file


def test_hit_objects_point_at_their_sliders():
    beatmap = beatmap_of("osu/diffcalc-test.osu")
    object_sliders = [o for o in beatmap.hit_objects if o.slider >= 0]
    assert len(object_sliders) == len(beatmap.sliders)
    assert all(0 <= o.slider < len(beatmap.sliders) for o in object_sliders)


def test_slider_geometry_is_exposed():
    slider = beatmap_of("osu/diffcalc-test.osu").sliders[0]
    assert slider.slides == 1
    assert slider.expected_length == 160.0
    assert len(slider.control_points) == 1
    assert len(slider.events) == 3
    assert len(slider.path) == len(slider.cumulative_lengths)


def test_a_spinner_is_centred():
    last = beatmap_of("osu/diffcalc-test.osu").hit_objects[-1]
    assert last.type & 8
    assert (last.position.x, last.position.y) == (256.0, 192.0)
    assert last.start_time == 102125.0
    assert last.end_time == 103000.0


def test_a_missing_file_is_the_os_error():
    with pytest.raises(FileNotFoundError):
        pppp.Beatmap.from_file(RESOURCES / "osu" / "does-not-exist.osu")


def test_the_errors_are_value_errors_under_one_base():
    assert issubclass(pppp.ParseError, ValueError)
    assert issubclass(pppp.ParseError, pppp.Error)
    assert issubclass(pppp.ModsError, ValueError)
    assert issubclass(pppp.ModsError, pppp.Error)
    assert pppp.ParseError.__module__ == "pppp"


def test_a_null_byte_in_the_path_is_an_error():
    with pytest.raises(ValueError):
        pppp.Beatmap.from_file(b"a\0b")


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
    attributes = pppp.Difficulty().calculate(beatmap_of(name))
    assert attributes.star_rating == pytest.approx(stars)
    assert attributes.max_combo == combo


def test_mods_change_the_calculation():
    beatmap = beatmap_of("fruits/2118524.osu")
    assert pppp.Difficulty(mods="HR").calculate(beatmap).star_rating == pytest.approx(
        4.308291009137178
    )


def test_stable_mod_bits_match_their_acronyms():
    beatmap = beatmap_of("fruits/2118524.osu")
    assert pppp.Difficulty(mods=16).calculate(beatmap) == pppp.Difficulty(mods="HR").calculate(
        beatmap
    )


def test_one_configuration_serves_many_maps():
    difficulty = pppp.Difficulty(mods="HR")
    catch = difficulty.calculate(beatmap_of("fruits/2118524.osu"))
    osu = difficulty.calculate(beatmap_of("osu/2785319.osu"))
    assert isinstance(catch, pppp.CatchDifficultyAttributes)
    assert isinstance(osu, pppp.OsuDifficultyAttributes)


def test_a_configuration_is_immutable():
    difficulty = pppp.Difficulty(mods="HR")
    with pytest.raises(dataclasses.FrozenInstanceError):
        difficulty.mods = "DT"  # type: ignore[misc]
    assert dataclasses.replace(difficulty, mods="DT").mods == "DT"


def test_the_ruleset_can_be_forced():
    attributes = pppp.Difficulty(ruleset=pppp.Ruleset.TAIKO).calculate(beatmap_of("osu/2785319.osu"))
    assert isinstance(attributes, pppp.TaikoDifficultyAttributes)
    assert attributes.ruleset is pppp.Ruleset.TAIKO
    assert attributes.star_rating == pytest.approx(4.752572620626138)
    assert attributes.mechanical_difficulty > 0.0


def test_the_attributes_are_the_live_ruleset_only():
    attributes = pppp.Difficulty().calculate(beatmap_of("taiko/1028484.osu"))
    assert type(attributes) is pppp.TaikoDifficultyAttributes
    assert not hasattr(attributes, "osu")


def test_the_attributes_take_a_match_statement():
    match pppp.Difficulty().calculate(beatmap_of("osu/2785319.osu")):
        case pppp.OsuDifficultyAttributes(star_rating=stars, max_combo=combo):
            assert stars == pytest.approx(6.004027372552197)
            assert combo == 909
        case _:
            pytest.fail("expected osu!standard attributes")


def test_an_unknown_mod_is_a_mods_error():
    beatmap = beatmap_of("osu/2785319.osu")
    with pytest.raises(pppp.ModsError, match="invalid mod specification") as error:
        pppp.Difficulty(mods="XX").calculate(beatmap)
    assert isinstance(error.value, ValueError)


def test_mods_of_another_type_are_a_type_error():
    beatmap = beatmap_of("osu/2785319.osu")
    with pytest.raises(TypeError):
        pppp.Difficulty(mods=1.5).calculate(beatmap)  # type: ignore[arg-type]


def test_a_beatmap_is_required():
    with pytest.raises(TypeError):
        pppp.Difficulty().calculate("not a beatmap")  # type: ignore[arg-type]


def test_performance_of_osu_matches_the_pinned_pp():
    beatmap = beatmap_of("osu/2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    assert isinstance(difficulty, pppp.OsuDifficultyAttributes)
    # The score carries the slider tails, as the library's own test does.
    play = pppp.Performance(
        max_combo=909,
        accuracy=1.0,
        statistics={
            pppp.HitResult.GREAT: 601,
            pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count,
        },
    ).calculate(beatmap)
    assert isinstance(play, pppp.OsuPerformanceAttributes)
    assert play.total == pytest.approx(316.5901855625614)
    assert play.aim == pytest.approx(148.75278891878943)
    assert play.speed == pytest.approx(61.34653468094172)
    assert play.accuracy == pytest.approx(98.99847982709288)
    assert play.reading == pytest.approx(2.2291238201795176)
    assert play.score_based_estimated_miss_count is None


def test_performance_of_taiko_matches_the_pinned_pp():
    beatmap = beatmap_of("taiko/1028484.osu")
    play = pppp.Performance(
        max_combo=289, accuracy=1.0, statistics={pppp.HitResult.GREAT: 289}
    ).calculate(beatmap)
    assert isinstance(play, pppp.TaikoPerformanceAttributes)
    assert play.ruleset is pppp.Ruleset.TAIKO
    assert play.total == pytest.approx(130.26636361095524)
    assert play.difficulty == pytest.approx(33.48488833057447)
    assert play.accuracy == pytest.approx(96.78147528038076)
    assert play.estimated_unstable_rate == pytest.approx(146.3238357972284)


def test_performance_of_catch_matches_the_pinned_pp():
    beatmap = beatmap_of("fruits/2118524.osu")
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
    assert play.ruleset is pppp.Ruleset.CATCH
    assert play.total == pytest.approx(112.72215339177879)


def test_performance_of_mania_matches_the_pinned_pp():
    beatmap = beatmap_of("mania/1638954.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    play = pppp.Performance(
        max_combo=difficulty.max_combo, accuracy=1.0, statistics={pppp.HitResult.PERFECT: 715}
    ).calculate(beatmap)
    assert play.ruleset is pppp.Ruleset.MANIA
    assert play.total == pytest.approx(108.92297471705167)


def test_performance_takes_the_combo_and_misses_it_is_given():
    play = pppp.Performance(max_combo=909, accuracy=1.0, misses=0).calculate(
        beatmap_of("osu/2785319.osu")
    )
    assert play.total > 0.0


def test_a_statistics_key_outside_the_hit_results_is_an_error():
    with pytest.raises(ValueError):
        pppp.Performance(statistics={99: 1}).calculate(  # type: ignore[dict-item]
            beatmap_of("osu/2785319.osu")
        )


def test_performance_from_attributes_matches_the_map():
    beatmap = beatmap_of("osu/2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    assert isinstance(difficulty, pppp.OsuDifficultyAttributes)
    performance = pppp.Performance(
        max_combo=909,
        accuracy=1.0,
        statistics={
            pppp.HitResult.GREAT: 601,
            pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count,
        },
    )
    from_map = performance.calculate(beatmap)
    from_attributes = performance.calculate(beatmap, difficulty)
    assert from_attributes == from_map


def test_attributes_of_another_type_are_a_type_error():
    with pytest.raises(TypeError, match="expected difficulty attributes"):
        pppp.Performance().calculate(
            beatmap_of("osu/2785319.osu"), "not attributes"  # type: ignore[arg-type]
        )


@pytest.mark.parametrize(
    ("name", "mods", "ruleset", "count", "middle", "last"),
    [
        ("osu/diffcalc-test.osu", "NM", pppp.Ruleset.OSU, 124, (18000, 5.727266936277771, 63),
         (103000, 6.524323005451468, 239)),
        ("osu/2785319.osu", "HD,DT", pppp.Ruleset.OSU, 601, (61309.235290751734, 8.272126401328721, 433),
         (115486.23529075173, 8.898179651893287, 909)),
        ("taiko/diffcalc-test.osu", "DT", pppp.Ruleset.TAIKO, 238, (21624, 4.397186455763557, 118),
         (53000, 4.455142137225538, 200)),
        ("fruits/diffcalc-test.osu", "DT", pppp.Ruleset.CATCH, 93, (14500, 4.27412411307246, 47),
         (45250, 5.152717389780087, 127)),
        ("mania/diffcalc-test.osu", "NM", pppp.Ruleset.MANIA, 137, (16250, 1.8228476946125378, 69),
         (30500, 2.3493769750220914, 242)),
        ("osu/2785319.osu", "4K", pppp.Ruleset.MANIA, 838, (61662, 2.5718820820666117, 530),
         (115486, 2.653795415351293, 1072)),
    ],
)
def test_the_timed_attributes_match_upstream(name, mods, ruleset, count, middle, last):
    timed = pppp.Difficulty(mods=mods, ruleset=ruleset).calculate_timed(beatmap_of(name))
    assert len(timed) == count
    for entry, (time, stars, combo) in ((timed[count // 2], middle), (timed[-1], last)):
        assert isinstance(entry, pppp.TimedDifficultyAttributes)
        assert entry.time == time
        assert entry.attributes.star_rating == pytest.approx(stars, rel=1e-6)
        assert entry.attributes.max_combo == combo
        assert entry.attributes.ruleset is ruleset


def test_the_osu_strain_graph_matches_upstream():
    strains = pppp.Difficulty(mods="HD,FL").strains(beatmap_of("osu/diffcalc-test.osu"))
    assert isinstance(strains, pppp.OsuStrains)
    assert strains.ruleset is pppp.Ruleset.OSU
    assert strains.start_time == 800
    assert strains.section_length == 400
    assert len(strains.aim) == len(strains.flashlight) == 254
    assert strains.aim[46] == pytest.approx(522.4551066294925, rel=1e-6)
    assert strains.speed[19] == pytest.approx(62.17504943719141, rel=1e-6)
    assert strains.flashlight[18] == pytest.approx(41.02690817661656, rel=1e-6)


@pytest.mark.parametrize(
    ("name", "mods", "ruleset", "kind", "start_time", "section_length", "series", "count", "index", "value"),
    [
        ("taiko/diffcalc-test.osu", "DT", pppp.Ruleset.TAIKO, pppp.TaikoStrains, 0, 600, "stamina", 89, 32,
         8.807433788330115),
        ("fruits/diffcalc-test.osu", "DT", pppp.Ruleset.CATCH, pppp.CatchStrains, 0, 1125, "movement", 41, 11,
         0.35301054964344125),
        ("mania/diffcalc-test.osu", "NM", pppp.Ruleset.MANIA, pppp.ManiaStrains, 400, 400, "strain", 76, 70,
         16.8859847732691),
        ("osu/2785319.osu", "4K", pppp.Ruleset.MANIA, pppp.ManiaStrains, 2800, 400, "strain", 282, 195,
         15.725755965531633),
    ],
)
def test_the_strain_graph_matches_upstream(
    name, mods, ruleset, kind, start_time, section_length, series, count, index, value
):
    strains = pppp.Difficulty(mods=mods, ruleset=ruleset).strains(beatmap_of(name))
    assert isinstance(strains, kind)
    assert strains.start_time == start_time
    assert strains.section_length == section_length
    values = getattr(strains, series)
    assert len(values) == count
    assert values[index] == pytest.approx(value, rel=1e-6)


def test_strain_graphs_match_by_pattern():
    match pppp.Difficulty(mods="DT").strains(beatmap_of("osu/2785319.osu")):
        case pppp.OsuStrains(start_time=start, flashlight=flashlight):
            assert start == 2400
            assert flashlight == []
        case _:
            pytest.fail("expected osu! strains")
