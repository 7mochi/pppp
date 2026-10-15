#!/usr/bin/env python3

from __future__ import annotations

import json
import os
import subprocess
import tempfile
import xml.etree.ElementTree as ElementTree

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.dirname(os.path.dirname(HERE))
REFERENCE = os.path.join(PROJECT, "tests", "reference", "upstream")
REFERENCE_DLL = os.path.join(REFERENCE, "bin", "Release", "net8.0", "reference.dll")
REFERENCE_CSPROJ = os.path.join(REFERENCE, "upstream.csproj")

RESOURCES = os.path.join(PROJECT, "tests", "resources")
ROWS = os.path.join(HERE, "rows_to_cover.json")

PARITY_JSON = os.path.join(HERE, "parity.json")

MODE_NAMES = {0: "osu", 1: "taiko", 2: "catch", 3: "mania"}

SCORE_FIELDS = {
    "osu": [("great", "count_great", "int"), ("ok", "count_ok", "int"), ("meh", "count_meh", "int"),
            ("miss", "count_miss", "int"), ("combo", "max_combo", "int"),
            ("legacy_total_score", "legacy_total_score", "int"), ("accuracy", "accuracy", "double")],
    "taiko": [("great", "count_great", "int"), ("ok", "count_ok", "int"), ("miss", "count_miss", "int"),
              ("combo", "max_combo", "int")],
    "catch": [("fruits", "count_great", "int"), ("droplets", "count_large_tick_hit", "int"),
              ("tiny_droplets", "count_small_tick_hit", "int"),
              ("tiny_misses", "count_small_tick_miss", "int"), ("miss", "count_miss", "int"),
              ("combo", "max_combo", "int")],
    "mania": [("perfect", "count_perfect", "int"), ("great", "count_great", "int"),
              ("good", "count_good", "int"), ("ok", "count_ok", "int"), ("meh", "count_meh", "int"),
              ("miss", "count_miss", "int"), ("combo", "max_combo", "int")],
}


def reference_version():
    root = ElementTree.parse(REFERENCE_CSPROJ).getroot()
    pins = {package.get("Include"): package.get("Version")
            for package in root.iter("PackageReference")}
    if len(set(pins.values())) != 1:
        raise SystemExit("upstream.csproj doesn't pin one version: %r" % pins)
    return next(iter(pins.values()))


def dotnet(arguments):
    command = ["dotnet"] + arguments
    if not os.environ.get("IN_NIX_SHELL"):
        command = ["nix", "develop", "--command"] + command
    return subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
        check=True,
    ).stdout


_built = False


def run_reference(rows_path):
    global _built
    if not _built:
        dotnet(["build", "-c", "Release", REFERENCE])
        _built = True

    descriptor, expected_path = tempfile.mkstemp(suffix=".json")
    os.close(descriptor)
    try:
        dotnet([REFERENCE_DLL, rows_path, RESOURCES, expected_path])
        with open(expected_path) as expected_file:
            return json.load(expected_file)
    finally:
        os.unlink(expected_path)


def score_value(row, key):
    value = row["score"].get(key)
    if value is not None:
        return str(value)
    return "-1" if key == "accuracy" else "0"


def mods_spec(mods):
    return ",".join(mod if isinstance(mod, str)
                    else mod["acronym"] + "".join(":%s=%s" % (key, value) for key, value in mod["settings"].items())
                    for mod in mods)


def ordered(entries, values):
    return {entry["name"]: values.get(entry["name"]) for entry in entries}


def score(mode, row):
    out = {}
    for name, key, ctype in SCORE_FIELDS[mode]:
        value = score_value(row, key)
        out[name] = int(value.split(".")[0]) if ctype == "int" else float(value)
    return out


def tables(schema, rows):
    decode = [{"map": row["map"].lstrip("/"), "decode": ordered(schema["decode"], row["decode"])}
              for row in rows]
    difficulty = {}
    performance = {}
    for mode_id, mode in MODE_NAMES.items():
        difficulty[mode] = []
        performance[mode] = []
        for row in rows:
            if row["ruleset"] != mode_id:
                continue
            case = {"name": row["case"], "map": row["map"].lstrip("/"), "mods": mods_spec(row["mods"])}
            difficulty[mode].append(dict(case, attributes=ordered(schema["difficulty"][str(mode_id)],
                                                                  row["difficulty"])))
            if "performance" in row:
                performance[mode].append(dict(case, score=score(mode, row),
                                              attributes=ordered(schema["performance"][str(mode_id)],
                                                                 row["performance"])))
    return {"version": reference_version(), "decode": decode, "difficulty": difficulty,
            "performance": performance}


def main():
    data = run_reference(ROWS)
    with open(PARITY_JSON, "w") as out:
        json.dump(tables(data["schema"], data["rows"]), out, indent=2)
        out.write("\n")


if __name__ == "__main__":
    main()
