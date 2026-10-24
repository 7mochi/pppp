"""The binding's behaviour, case for case with the Node suite."""

from __future__ import annotations

import json
import os
import pathlib
import platform
import subprocess
import sys

import pytest

import pppp

HERE = pathlib.Path(__file__).resolve().parent
RESOURCES = HERE.parent.parent / "tests" / "resources"
PARITY = json.loads((HERE.parent.parent / "tests" / "parity" / "parity.json").read_text())


def beatmap_of(name: str) -> pppp.Beatmap:
    return pppp.Beatmap.from_file(RESOURCES / name)


def approx(actual: float, expected: float, epsilon: float) -> bool:
    return abs(actual - expected) < epsilon * (1.0 + max(abs(actual), abs(expected)))


def test_the_version_is_a_version():
    assert pppp.__version__.count(".") == 2


def test_from_file_reads_the_model():
    beatmap = beatmap_of("osu/diffcalc-test.osu")

    assert isinstance(beatmap, pppp.Beatmap)
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


def test_from_file_resolves_to_the_same_beatmap():
    file = RESOURCES / "osu/diffcalc-test.osu"

    assert pppp.Difficulty().calculate(pppp.Beatmap.from_file(file)) == pppp.Difficulty().calculate(
        pppp.Beatmap.from_file(file)
    )


def test_from_bytes_matches_the_file():
    file = RESOURCES / "osu/diffcalc-test.osu"
    data = file.read_bytes()

    assert pppp.Difficulty().calculate(pppp.Beatmap.from_bytes(data)) == pppp.Difficulty().calculate(
        pppp.Beatmap.from_file(file)
    )


def test_a_missing_file_is_the_os_error():
    with pytest.raises(FileNotFoundError):
        pppp.Beatmap.from_file(RESOURCES / "osu" / "does-not-exist.osu")


def test_from_bytes_takes_bytes_only():
    with pytest.raises(TypeError):
        pppp.Beatmap.from_bytes("osu file format v14")  # type: ignore[arg-type]


def test_a_beatmap_is_not_constructed_directly():
    with pytest.raises(TypeError):
        pppp.Beatmap()  # type: ignore[call-arg]


def test_hit_objects_point_at_their_sliders():
    beatmap = beatmap_of("osu/diffcalc-test.osu")
    object_sliders = [item for item in beatmap.hit_objects if item.slider >= 0]

    assert len(object_sliders) == len(beatmap.sliders)
    for item in object_sliders:
        assert 0 <= item.slider < len(beatmap.sliders)


def test_the_slider_geometry_is_exposed():
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


def test_one_configuration_serves_many_maps():
    difficulty = pppp.Difficulty(mods="HR")

    assert isinstance(difficulty.calculate(beatmap_of("fruits/2118524.osu")), pppp.CatchDifficultyAttributes)
    assert isinstance(difficulty.calculate(beatmap_of("osu/2785319.osu")), pppp.OsuDifficultyAttributes)


@pytest.mark.parametrize(
    ("name", "stars", "combo"),
    [
        ("osu/2785319.osu", 6.004027372552197, 909),
        ("taiko/1028484.osu", 2.9144215641617333, 289),
        ("fruits/2118524.osu", 3.2340182503279706, 730),
        ("mania/1638954.osu", 3.358304846842773, 956),
    ],
)
def test_the_difficulty_matches_the_pinned_stars(name: str, stars: float, combo: int):
    attributes = pppp.Difficulty().calculate(beatmap_of(name))

    assert attributes.star_rating == pytest.approx(stars)
    assert attributes.max_combo == combo


def test_mods_change_the_calculation():
    attributes = pppp.Difficulty(mods="HR").calculate(beatmap_of("fruits/2118524.osu"))

    assert attributes.star_rating == pytest.approx(4.308291009137178)


def test_classic_scores_on_the_lazer_algorithm():
    beatmap = beatmap_of("osu/2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    score = dict(
        max_combo=909,
        accuracy=1.0,
        statistics={pppp.HitResult.GREAT: 601, pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count},
    )

    classic = pppp.Performance(mods="CL", **score).calculate(beatmap)
    twice = pppp.Performance(mods="CL,CL", **score).calculate(beatmap)
    lazer = pppp.Performance(**score).calculate(beatmap)

    assert classic.total == pytest.approx(298.5325815982227)
    assert twice.total == pytest.approx(classic.total)
    assert lazer.total == pytest.approx(316.5901855625614)


def test_classic_works_from_the_option_instead_of_a_mod():
    beatmap = beatmap_of("osu/2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    score = dict(
        max_combo=909,
        accuracy=1.0,
        statistics={pppp.HitResult.GREAT: 601, pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count},
    )

    assert pppp.Performance(classic=True, **score).calculate(beatmap).total == pytest.approx(
        298.5325815982227
    )


def test_stable_mod_bits_match_their_acronyms():
    beatmap = beatmap_of("fruits/2118524.osu")

    assert pppp.Difficulty(mods=16).calculate(beatmap) == pppp.Difficulty(mods="HR").calculate(beatmap)


def test_the_ruleset_can_be_forced():
    attributes = pppp.Difficulty(ruleset=pppp.Ruleset.TAIKO).calculate(beatmap_of("osu/2785319.osu"))

    assert attributes.ruleset == pppp.Ruleset.TAIKO
    assert attributes.star_rating == pytest.approx(4.752572620626138)
    assert attributes.mechanical_difficulty > 0.0


def test_the_attributes_are_the_live_ruleset_only():
    attributes = pppp.Difficulty().calculate(beatmap_of("taiko/1028484.osu"))

    assert attributes.ruleset == pppp.Ruleset.TAIKO
    assert not hasattr(attributes, "osu")
    assert not hasattr(attributes, "aim_difficulty")


def test_an_unknown_mod_is_an_error_with_a_code():
    with pytest.raises(pppp.ModsError, match="invalid mod specification"):
        pppp.Difficulty(mods="XX").calculate(beatmap_of("osu/2785319.osu"))


def test_a_beatmap_is_required():
    with pytest.raises(TypeError):
        pppp.Difficulty().calculate("not a beatmap")  # type: ignore[arg-type]


def test_a_ruleset_outside_the_four_is_an_error():
    with pytest.raises(pppp.Error):
        pppp.Difficulty(ruleset=9).calculate(beatmap_of("osu/2785319.osu"))  # type: ignore[arg-type]


def test_the_performance_of_osu_matches_the_pinned_pp():
    beatmap = beatmap_of("osu/2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    play = pppp.Performance(
        max_combo=909,
        accuracy=1.0,
        statistics={pppp.HitResult.GREAT: 601, pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count},
    ).calculate(beatmap)

    assert play.ruleset == pppp.Ruleset.OSU
    assert play.total == pytest.approx(316.5901855625614)
    assert play.aim == pytest.approx(148.75278891878943)
    assert play.speed == pytest.approx(61.34653468094172)
    assert play.accuracy == pytest.approx(98.99847982709288)
    assert play.reading == pytest.approx(2.2291238201795176)
    assert play.score_based_estimated_miss_count is None


def test_the_performance_of_taiko_matches_the_pinned_pp():
    play = pppp.Performance(
        max_combo=289, accuracy=1.0, statistics={pppp.HitResult.GREAT: 289}
    ).calculate(beatmap_of("taiko/1028484.osu"))

    assert play.ruleset == pppp.Ruleset.TAIKO
    assert play.total == pytest.approx(130.26636361095524)
    assert play.difficulty == pytest.approx(33.48488833057447)
    assert play.accuracy == pytest.approx(96.78147528038076)
    assert play.estimated_unstable_rate == pytest.approx(146.3238357972284)


def test_the_performance_of_catch_matches_the_pinned_pp():
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

    assert play.ruleset == pppp.Ruleset.CATCH
    assert play.total == pytest.approx(112.72215339177879)


def test_the_performance_of_mania_matches_the_pinned_pp():
    beatmap = beatmap_of("mania/1638954.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    play = pppp.Performance(
        max_combo=difficulty.max_combo,
        accuracy=1.0,
        statistics={pppp.HitResult.PERFECT: 715},
    ).calculate(beatmap)

    assert play.ruleset == pppp.Ruleset.MANIA
    assert play.total == pytest.approx(108.92297471705167)
    assert play.difficulty == pytest.approx(108.92297471705167)


def test_the_performance_takes_the_combo_and_misses_it_is_given():
    play = pppp.Performance(max_combo=909, accuracy=1.0, misses=0).calculate(
        beatmap_of("osu/2785319.osu")
    )

    assert play.total > 0.0


def test_a_statistics_key_outside_the_hit_results_is_an_error():
    with pytest.raises(ValueError):
        pppp.Performance(statistics={99: 1}).calculate(  # type: ignore[dict-item]
            beatmap_of("osu/2785319.osu")
        )


def test_the_performance_from_attributes_matches_the_map():
    beatmap = beatmap_of("osu/2785319.osu")
    difficulty = pppp.Difficulty().calculate(beatmap)
    performance = pppp.Performance(
        max_combo=909,
        accuracy=1.0,
        statistics={pppp.HitResult.GREAT: 601, pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count},
    )

    assert performance.calculate(beatmap, difficulty) == performance.calculate(beatmap)


@pytest.mark.parametrize(
    ("name", "mods", "ruleset", "count", "middle", "last"),
    [
        ("osu/diffcalc-test.osu", "NM", 0, 124, (18000, 5.727266936277771, 63), (103000, 6.524323005451468, 239)),
        ("osu/2785319.osu", "HD,DT", 0, 601, (61309.235290751734, 8.272126401328721, 433), (115486.23529075173, 8.898179651893287, 909)),
        ("taiko/diffcalc-test.osu", "DT", 1, 238, (21624, 4.397186455763557, 118), (53000, 4.455142137225538, 200)),
        ("fruits/diffcalc-test.osu", "DT", 2, 93, (14500, 4.27412411307246, 47), (45250, 5.152717389780087, 127)),
        ("mania/diffcalc-test.osu", "NM", 3, 137, (16250, 1.8228476946125378, 69), (30500, 2.3493769750220914, 242)),
        ("osu/2785319.osu", "4K", 3, 838, (61662, 2.5718820820666117, 530), (115486, 2.653795415351293, 1072)),
    ],
)
def test_the_timed_attributes_match_upstream(name, mods, ruleset, count, middle, last):
    timed = pppp.Difficulty(mods=mods, ruleset=ruleset).calculate_timed(beatmap_of(name))

    assert len(timed) == count

    for entry, (time, stars, combo) in (
        (timed[count // 2], middle),
        (timed[count - 1], last),
    ):
        assert entry.time == time
        assert entry.attributes.star_rating == pytest.approx(stars)
        assert entry.attributes.max_combo == combo
        assert entry.attributes.ruleset == ruleset


@pytest.mark.parametrize(
    ("name", "mods", "ruleset", "start_time", "section_length", "series", "count", "index", "value"),
    [
        ("osu/diffcalc-test.osu", "HD,FL", 0, 800, 400, "flashlight", 254, 18, 41.02690817661656),
        ("osu/2785319.osu", "DT", 0, 2400, 600, "aim_no_sliders", 189, 66, 517.1805782727978),
        ("taiko/diffcalc-test.osu", "DT", 1, 0, 600, "single_colour_stamina", 89, 6, 4.473586847131765),
        ("fruits/diffcalc-test.osu", "DT", 2, 0, 1125, "movement", 41, 11, 0.35301054964344125),
        ("mania/diffcalc-test.osu", "NM", 3, 400, 400, "strain", 76, 70, 16.8859847732691),
        ("osu/2785319.osu", "4K", 3, 2800, 400, "strain", 282, 195, 15.725755965531633),
    ],
)
def test_the_strain_graph_matches_upstream(
    name, mods, ruleset, start_time, section_length, series, count, index, value
):
    strains = pppp.Difficulty(mods=mods, ruleset=ruleset).calculate_strains(beatmap_of(name))

    assert strains.ruleset == ruleset
    assert strains.start_time == start_time
    assert strains.section_length == section_length
    assert len(getattr(strains, series)) == count
    assert getattr(strains, series)[index] == pytest.approx(value)


def _backend_with(variant: str) -> str:
    out = subprocess.run(
        [sys.executable, "-c", "import pppp; print(pppp.backend)"],
        env={**os.environ, "FOSU_BACKEND": variant},
        capture_output=True,
        text=True,
        check=True,
    )
    return out.stdout.strip()


def test_backend_reports_the_engine():
    if os.environ.get("FOSU_BACKEND") == "scalar":
        assert pppp.backend == "scalar"
        return

    runnable = next(
        (variant for variant in ("avx2", "neon") if _backend_with(variant) == variant), "scalar"
    )

    assert pppp.backend == runnable


def test_backend_ships_beside_the_library():
    here = pppp.library.library_path().parent
    engines = list(here.glob("*fosu_engine_*"))

    if platform.machine().lower() in {"x86_64", "amd64", "arm64", "aarch64"}:
        assert engines, f"no engine beside {here}"


def test_backend_scalar_override():
    assert _backend_with("scalar") == "scalar"


def test_the_decode_parity_rows_match_upstream():
    for row in PARITY["decode"]:
        beatmap = beatmap_of(row["map"])
        expected = row["decode"]
        uninherited = len([point for point in beatmap.timing_points if point.uninherited])

        assert beatmap.format_version == expected["format"]
        assert beatmap.mode == expected["mode"]
        assert len(beatmap.hit_objects) == expected["hit_objects"]
        assert uninherited == expected["uninherited_points"]
        assert len(beatmap.breaks) == expected["breaks"]

        for actual, wanted in (
            (beatmap.difficulty.approach_rate, expected["ar"]),
            (beatmap.difficulty.overall_difficulty, expected["od"]),
            (beatmap.difficulty.circle_size, expected["cs"]),
            (beatmap.difficulty.drain_rate, expected["hp"]),
            (beatmap.difficulty.slider_multiplier, expected["slider_multiplier"]),
            (beatmap.difficulty.slider_tick_rate, expected["slider_tick_rate"]),
            (beatmap.stack_leniency, expected["stack_leniency"]),
        ):
            assert approx(actual, wanted, 1e-6)


def _classic_accuracy(great: int, ok: int, meh: int, miss: int) -> float:
    total = great + ok + meh + miss

    return 1.0 if total <= 0 else (great * 300.0 + ok * 100.0 + meh * 50.0) / (total * 300.0)


def _osu_score(score: dict, difficulty: pppp.DifficultyAttributes) -> dict:
    accuracy = score["accuracy"]
    if accuracy < 0.0:
        accuracy = _classic_accuracy(score["great"], score["ok"], score["meh"], score["miss"])

    return dict(
        max_combo=score["combo"],
        accuracy=accuracy,
        statistics={
            pppp.HitResult.GREAT: score["great"],
            pppp.HitResult.OK: score["ok"],
            pppp.HitResult.MEH: score["meh"],
            pppp.HitResult.MISS: score["miss"],
            pppp.HitResult.SLIDER_TAIL_HIT: difficulty.slider_count,
        },
        legacy_total_score=score["legacy_total_score"] or None,
    )


def _taiko_score(score: dict, difficulty: pppp.DifficultyAttributes) -> dict:
    return dict(
        max_combo=score["combo"] or difficulty.max_combo,
        accuracy=(score["great"] * 300.0 + score["ok"] * 150.0)
        / ((score["great"] + score["ok"] + score["miss"]) * 300.0),
        statistics={
            pppp.HitResult.GREAT: score["great"],
            pppp.HitResult.OK: score["ok"],
            pppp.HitResult.MISS: score["miss"],
        },
    )


def _catch_score(score: dict, difficulty: pppp.DifficultyAttributes) -> dict:
    hits = score["fruits"] + score["droplets"] + score["tiny_droplets"]

    return dict(
        max_combo=score["combo"] or difficulty.max_combo,
        accuracy=hits / (hits + score["tiny_misses"] + score["miss"]),
        statistics={
            pppp.HitResult.GREAT: score["fruits"],
            pppp.HitResult.LARGE_TICK_HIT: score["droplets"],
            pppp.HitResult.SMALL_TICK_HIT: score["tiny_droplets"],
            pppp.HitResult.SMALL_TICK_MISS: score["tiny_misses"],
            pppp.HitResult.MISS: score["miss"],
        },
    )


def _mania_score(score: dict, difficulty: pppp.DifficultyAttributes) -> dict:
    total = score["perfect"] + score["great"] + score["good"] + score["ok"] + score["meh"] + score["miss"]
    weighted = (
        score["perfect"] * 305.0
        + score["great"] * 300.0
        + score["good"] * 200.0
        + score["ok"] * 100.0
        + score["meh"] * 50.0
    )

    return dict(
        max_combo=score["combo"] or difficulty.max_combo,
        accuracy=weighted / (total * 305.0),
        statistics={
            pppp.HitResult.PERFECT: score["perfect"],
            pppp.HitResult.GREAT: score["great"],
            pppp.HitResult.GOOD: score["good"],
            pppp.HitResult.OK: score["ok"],
            pppp.HitResult.MEH: score["meh"],
            pppp.HitResult.MISS: score["miss"],
        },
    )


@pytest.mark.parametrize("mode", ["osu", "taiko", "catch", "mania"])
def test_the_parity_difficulty_rows_match_upstream(mode: str):
    for row in PARITY["difficulty"][mode]:
        attributes = pppp.Difficulty(
            mods=row["mods"], ruleset=getattr(pppp.Ruleset, mode.upper())
        ).calculate(beatmap_of(row["map"]))

        assert approx(attributes.star_rating, row["attributes"]["star_rating"], 1e-6)
        assert attributes.max_combo == row["attributes"]["max_combo"]


def test_the_parity_performance_rows_match_upstream():
    scorers = {
        "osu": _osu_score,
        "taiko": _taiko_score,
        "catch": _catch_score,
        "mania": _mania_score,
    }

    for mode, scorer in scorers.items():
        for row in PARITY["performance"][mode]:
            beatmap = beatmap_of(row["map"])
            difficulty = pppp.Difficulty(
                mods=row["mods"], ruleset=getattr(pppp.Ruleset, mode.upper())
            ).calculate(beatmap)
            play = pppp.Performance(mods=row["mods"], **scorer(row["score"], difficulty)).calculate(
                beatmap, difficulty
            )

            assert approx(play.total, row["attributes"]["total"], 1e-9)
