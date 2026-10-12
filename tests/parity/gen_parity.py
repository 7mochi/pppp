#!/usr/bin/env python3

from __future__ import annotations

import datetime
import json
import os
import subprocess
import tempfile

try:
    from jinja2 import Environment, FileSystemLoader, StrictUndefined
except ImportError as error:
    raise SystemExit(
        "gen_parity.py needs jinja2: run it inside the dev shell (nix develop / direnv)."
    ) from error

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.dirname(os.path.dirname(HERE))
REFERENCE = os.path.join(PROJECT, "tests", "reference", "upstream")
REFERENCE_DLL = os.path.join(REFERENCE, "bin", "Release", "net10.0", "reference.dll")

PPDEV_PIN = "496c13e0749a0995779e5169c3ff4c4ffe2cf525"

RESOURCES = os.path.join(PROJECT, "tests", "resources")
ROWS = os.path.join(HERE, "rows_to_cover.json")
TEMPLATES = os.path.join(HERE, "templates")

DECODE_HEADER = os.path.join(HERE, "decode.h")
DIFFICULTY_HEADER = os.path.join(HERE, "difficulty.h")
PERFORMANCE_HEADER = os.path.join(HERE, "performance.h")

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

CPP_TYPES = {
    "decode": {"double": "float", "int": "int", "long": "int"},
    "difficulty": {"double": "float", "double?": "float", "int": "int", "long": "int"},
    "performance": {"double": "double", "double?": "double", "int": "int", "long": "int"},
}

PERFORMANCE_PREFIX = "pp_"

LEADING_FIELDS = {
    "decode": [("const char*", "map")],
    "difficulty": [("const char*", "name"), ("const char*", "map"), ("const char*", "mods")],
    "performance": [("const char*", "name"), ("const char*", "map"), ("const char*", "mods"),
                    ("const char*", "score")],
}


def reference_pin():
    root = os.path.dirname(PROJECT)
    osu = os.path.join(root, "osu")
    try:
        pin = subprocess.run(
            ["git", "-C", osu, "rev-parse", "HEAD"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            check=True,
        ).stdout.strip()
    except subprocess.CalledProcessError as error:
        raise SystemExit(
            "cannot read the pin of osu/: check out ppy/osu next to this workspace at pp-dev %s"
            % PPDEV_PIN
        ) from error
    if pin != PPDEV_PIN:
        raise SystemExit("osu/ is at %s, expected pp-dev %s" % (pin, PPDEV_PIN))
    return pin


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


def cpp_float(value):
    return repr(float(value))


def cpp_double(value):
    text = repr(float(value))
    return text[:-2] if text.endswith(".0") else text


def number(value, ctype):
    if ctype == "float":
        return "std::numeric_limits<float>::quiet_NaN()" if value is None else cpp_float(value) + "f"
    if ctype == "double":
        return "std::numeric_limits<double>::quiet_NaN()" if value is None else cpp_double(value)
    return str(value)


def score_value(row, key):
    value = row["score"].get(key)
    if value is not None:
        return str(value)
    # A score field that carries an optional in the port uses -1 for "absent", so that a present
    # zero (e.g. legacy_total_score = 0) stays distinguishable from a missing one.
    return "-1" if key in ("accuracy", "legacy_total_score") else "0"


def attribute_fields(section, entries):
    return [(CPP_TYPES[section][entry["type"]], entry["name"]) for entry in entries]


def decode_table(schema, rows):
    fields = LEADING_FIELDS["decode"] + attribute_fields("decode", schema["decode"])
    body = [[quote(row["map"])] + [number(row["decode"][entry["name"]], ctype)
                                   for entry, (ctype, _) in zip(schema["decode"], fields[1:])]
            for row in rows]
    return {"struct": "DecodeRow", "array": "decode_rows", "fields": fields, "rows": body}


def difficulty_tables(schema, by_mode):
    tables = []
    for mode_id, mode in MODE_NAMES.items():
        entries = schema["difficulty"][str(mode_id)]
        fields = LEADING_FIELDS["difficulty"] + attribute_fields("difficulty", entries)
        body = [[quote(row["case"]), quote("/" + row["map"].lstrip("/")), quote(",".join(row["mods"]))]
                + [number(row["difficulty"][entry["name"]], ctype)
                   for entry, (ctype, _) in zip(entries, fields[3:])]
                for row in by_mode.get(mode, [])]
        tables.append({"struct": mode.title() + "DiffRow", "array": mode + "_diff_rows",
                       "fields": fields, "rows": body})
    return tables


def performance_tables(schema, scored_by_mode):
    tables = []
    for mode_id, mode in MODE_NAMES.items():
        entries = schema["performance"][str(mode_id)]
        score_fields = SCORE_FIELDS[mode]
        fields = (LEADING_FIELDS["performance"]
                  + [(ctype, name) for name, _key, ctype in score_fields]
                  + [(CPP_TYPES["performance"][entry["type"]], PERFORMANCE_PREFIX + entry["name"])
                     for entry in entries])
        body = []
        for row in scored_by_mode.get(mode, []):
            values = [quote(row["case"]), quote("/" + row["map"].lstrip("/")), quote(",".join(row["mods"])),
                      quote(",".join("%s=%s" % (key, value) for key, value in row["score"].items()))]
            for _name, key, ctype in score_fields:
                value = score_value(row, key)
                values.append(value.split(".")[0] if ctype == "int" else value)
            for entry, (ctype, _) in zip(entries, fields[4 + len(score_fields):]):
                values.append(number(row["performance"].get(entry["name"]), ctype))
            body.append(values)
        tables.append({"struct": mode.title() + "PpRow", "array": mode + "_pp_rows",
                       "fields": fields, "rows": body})
    return tables


def quote(value):
    return '"%s"' % value


def guard_for(path):
    return "PPPP_TESTS_PARITY_" + os.path.basename(path).replace(".h", "").upper() + "_H"


def render(template, path, tables, includes):
    with open(path, "w") as out:
        out.write(template.render(
            version=reference_pin(),
            date=datetime.date.today().isoformat(),
            guard=guard_for(path),
            tables=tables,
            includes=includes,
        ))


def main():
    data = run_reference(ROWS)
    schema = data["schema"]
    rows = data["rows"]

    by_mode = {}
    scored_by_mode = {}
    for row in rows:
        mode = MODE_NAMES[row["ruleset"]]
        by_mode.setdefault(mode, []).append(row)
        if "performance" in row:
            scored_by_mode.setdefault(mode, []).append(row)

    environment = Environment(
        loader=FileSystemLoader(TEMPLATES),
        undefined=StrictUndefined,
        autoescape=False,
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )
    template = environment.get_template("parity.h.jinja")

    render(template, DECODE_HEADER, [decode_table(schema, rows)], [])
    render(template, DIFFICULTY_HEADER, difficulty_tables(schema, by_mode), ["limits"])
    render(template, PERFORMANCE_HEADER, performance_tables(schema, scored_by_mode), ["limits"])


if __name__ == "__main__":
    main()
