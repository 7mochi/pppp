# PerformancePoints++

Calculate osu! difficulty and performance for all four gamemodes in standard C++98.

- All four gamemodes, with the mods each one supports.
- Matches osu!lazer `2026.730.0`.
- Reads beatmaps with [fosu](https://github.com/cmyui/fast-osu-beatmap-parser)
  `de01f188f2be7831f619ca09882cf2f55b0c9720`.

## Build and install

xmake builds and installs the library:

```sh
xmake f -y -m release
xmake
xmake install -o <prefix>
```

It installs two pkg-config packages. `pppp` is the calculator. `pppp_fosu`
is the calculator plus the beatmap parser. To build a program, use the flags
from the package you need:

```sh
PKG_CONFIG_PATH=<prefix>/lib/pkgconfig pkg-config --cflags --libs pppp_fosu
c++ -std=c++98 $(pkg-config --cflags pppp_fosu) app.cpp $(pkg-config --libs pppp_fosu) -o app
```

You don't need a separate package for each internal part. The two packages
already include everything needed to build.

The parser has an optional faster version. To use it, copy
`libfosu_engine_avx2.so` (or the NEON one) from `<prefix>/lib` next to your
program. If you leave it out, the parser uses the slower version.

## Usage

```cpp
#include <pppp/backend.h>
#include <pppp/beatmap.h>
#include <pppp/pppp.h>

pppp::Beatmap beatmap;
pppp::Status status = beatmap.load_file("map.osu");    // or beatmap.load_buffer(data, size)
if (!status.ok()) { puts(status.message()); return 1; }

pppp::Mods mods;
if (!mods.parse("HDDT").ok()) return 1;                 // "DT:speed_change=1.4" for settings
// pppp::Mods::from_legacy(pppp::mods::LEGACY_MOD_HIDDEN, beatmap.mode) for osu!stable's bitmask
// mods.push_back(pppp::mods::MOD_HD) for a single mod

pppp::DifficultyAttributes diff_attrs;
if (!pppp::Difficulty().mods(mods).calculate(beatmap, diff_attrs).ok()) return 1;
if (const pppp::OsuDifficultyAttributes* osu = diff_attrs.osu()) printf("aim %.2f\n", osu->aim_difficulty);

pppp::ScoreInfo score;
score.mods = mods;
score.statistics[pppp::HIT_RESULT_GREAT] = 600;
score.statistics[pppp::HIT_RESULT_OK] = 1;
score.max_combo = diff_attrs.max_combo(); score.accuracy = 0.9994;

pppp::PerformanceAttributes pp_attrs;
if (!pppp::Performance(beatmap, diff_attrs).score(score).calculate(pp_attrs).ok()) return 1;

printf("%.2f* %.2fpp\n", diff_attrs.star_rating(), pp_attrs.total());

// The parser version that was loaded: pppp::EngineKind::SCALAR, AVX2 or NEON.
// pppp::backend() is pppp::EngineKind::AVX2 on an x86-64 host, for example.
```

## Options

| xmake option | Default | What it does |
| --- | --- | --- |
| `--pppp_with_fosu=n` | ON | builds the `.osu` parser and the code that connects it |
| `--pppp_isa=auto\|scalar\|avx2\|neon` | auto | selects the parser speed |
| `--pppp_build_tests=n` | ON | builds the tests (downloads doctest and cJSON) |
| `--pppp_warnings_as_errors=y` | OFF | treats compiler warnings as errors |
| `--doctest_dir=<dir>` | empty | uses a local doctest.h |
| `--cjson_dir=<dir>` | empty | uses local cJSON files |

## Working on the project

The tools come from the Nix development shell:

```sh
nix develop        # compilers, xmake, pkg-config, Python, .NET and pre-commit
make test-auto     # build, run the tests, and test the installed package
make check         # formatting, headers, and code checks
make release       # optimized build without tests
```

Formatting and file checks also run through
[pre-commit](https://pre-commit.com). The shell installs the hooks. To run them
by hand:

```sh
pre-commit run --all-files
```

`clang-format` and `nixfmt` come from the shell. Outside it, the hooks fall back
to `nix develop`.

## AI Disclaimer

The port was done with AI assistance (DeepSeek V4.1 Flash and Opus 5), working file by file against ppy/osu. I have gone through most of the code to check that it's correct and to fix what isn't. Not all of it though, some files have not been checked yet and I'm still working through the rest. If you find something, an issue is welcome.

If you would rather not use something written with AI assistance, there are other options for the
same problem:
- [osu-native](https://github.com/minisbett/osu-native) with my bindings:
  [osu-native-py](https://github.com/7mochi/osu-native-py),
  [osu-native-go](https://github.com/7mochi/osu-native-go) and
  [osu-native-jar](https://github.com/7mochi/osu-native-jar).
- [rosu-pp](https://github.com/MaxOhn/rosu-pp) with
  [rosu-pp-py](https://github.com/MaxOhn/rosu-pp-py),
  [rosu-pp-ffi](https://github.com/MaxOhn/rosu-pp-ffi) and
  [rosu-pp-js](https://github.com/MaxOhn/rosu-pp-js).
