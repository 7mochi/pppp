from __future__ import annotations

import pathlib

import pytest

import pppp
from tests.conftest import RESOURCES
from pppp._ctypes import _native


def test_version_format():
    assert pppp.__version__.count(".") == 2


def test_backend_name():
    assert pppp.backend in {"scalar", "avx2", "neon"}


def test_library_path():
    path = pppp.library.library_path()

    assert isinstance(path, pathlib.Path)
    assert path.exists()
    assert path.name.endswith((".so", ".dylib", ".dll"))


def test_allocation_status():
    with pytest.raises(MemoryError):
        pppp.native.check_status(_native.Pppp_Allocation)


def test_success_status():
    pppp.native.check_status(0)


def test_file_metadata():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    assert beatmap.title == "re[in]flaw"
    assert beatmap.artist == "MYUKKE."
    assert beatmap.version == "toybot's Expert"
    assert beatmap.beatmap_id == 2785319
    assert beatmap.format_version == 14
    assert beatmap.mode == 0
    assert beatmap.stack_leniency == pytest.approx(0.5)
    assert beatmap.difficulty.circle_size == pytest.approx(4.5)


def test_bytes_and_text():
    data = (RESOURCES / "osu" / "2785319.osu").read_bytes()
    assert pppp.Beatmap.from_bytes(data).beatmap_id == 2785319
    assert pppp.Beatmap.from_text(data.decode()).beatmap_id == 2785319


def test_missing_file():
    with pytest.raises(FileNotFoundError):
        pppp.Beatmap.from_file(RESOURCES / "osu" / "does-not-exist.osu")


def test_bytes_type():
    with pytest.raises(TypeError):
        pppp.Beatmap.from_bytes("osu file format v14")


def test_text_type():
    with pytest.raises(TypeError):
        pppp.Beatmap.from_text(b"osu file format v14")


def test_closed_beatmap_property():
    beatmap = pppp.Beatmap.from_file(RESOURCES / "osu" / "2785319.osu")
    with beatmap as opened:
        assert opened.beatmap_id == 2785319
    with pytest.raises(ValueError):
        _ = beatmap.title


def test_ruleset_values():
    assert [int(item) for item in pppp.Ruleset] == [0, 1, 2, 3]


def test_hit_result_values():
    assert pppp.HitResult.GREAT == 5
    assert pppp.HitResult.SLIDER_TAIL_HIT == 16
    assert pppp.HitResult.LEGACY_COMBO_INCREASE == 17
