# PerformancePoints++

Calculate osu! difficulty and performance attributes for all four gamemodes in ISO C++98.

- All four gamemodes, with every mod the game lets each carry.
- Ported from [osu!lazer](https://github.com/ppy/osu) at `4f649c51b1b911361244e253e3469b2fb456f00e`, and from
[fosu](https://github.com/cmyui/fast-osu-beatmap-parser)
`de01f188f2be7831f619ca09882cf2f55b0c9720` for the parser.
- Python bindings written against CPython's stable ABI, so one `cp310-abi3` wheel serves every CPython from 3.10.

## Install

TODO

## Usage

```cpp
#include <pppp/beatmap.h>
#include <pppp/pppp.h>

pppp::beatmaps::Beatmap beatmap;
if (pppp::beatmaps::from_file(beatmap, "map.osu") != pppp::Result::OK) return 1;

pppp::mods::Mod mods[16];
int mod_count = pppp::mods::mod_from_acronyms(mods, 16, "HDDT");
// pppp::mods::mod_from_acronyms(mods, 16, "DT:speed_change=1.4") for settings
// pppp::mods::mod_from_legacy(mods, 16, pppp::mods::LEGACY_MOD_HIDDEN) for osu!stable's bitmask
// pppp::mods::mod_make(&mods[0], pppp::mods::MOD_HD) for a single mod

pppp::DifficultyAttributes diff_attrs = pppp::Difficulty().mods(mods, mod_count).calculate(beatmap);

pppp::common::ScoreInfo score;
score.mods = mods; score.mod_count = mod_count;
score.statistics[pppp::common::HIT_RESULT_GREAT] = 600;
score.statistics[pppp::common::HIT_RESULT_OK] = 1;
score.max_combo = diff_attrs.max_combo(); score.accuracy = 0.9994;

pppp::PerformanceAttributes pp_attrs =
    pppp::Performance(beatmap, diff_attrs).mods(mods, mod_count).state(score).calculate();

printf("%.2f* %.2fpp\n", diff_attrs.star_rating(), pp_attrs.total());
```

## Python

```python
import pppp

beatmap = pppp.from_file("map.osu")

difficulty = pppp.Difficulty(beatmap).mods("HD,DT").calculate()
performance = pppp.Performance(beatmap, combo=789, accuracy=0.992, misses=2).calculate()

print(difficulty.star_rating, performance.total)
```

## Options

| CMake option | Default | What it does |
| --- | --- | --- |
| `PPPP_WITH_FOSU` | ON | the parser adapter and the vendored parser |
| `PPPP_BUILD_TESTS` | top-level only | the suites (fetches doctest) |
| `PPPP_BUILD_PYTHON` | OFF | the Python extension module |
| `PPPP_INSTALL` | top-level only | the `find_package(pppp CONFIG)` package |
| `PPPP_WARNINGS_AS_ERRORS` | OFF | `-Werror` on the library's warnings |

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
