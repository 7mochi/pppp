# PerformancePoints++

Calculate osu! difficulty and performance attributes for all four gamemodes in ISO C++98.

- All four gamemodes, with every mod the game lets each carry.
- Ported from [osu!lazer](https://github.com/ppy/osu) at `4f649c51b1b911361244e253e3469b2fb456f00e`, and from
[fosu](https://github.com/cmyui/fast-osu-beatmap-parser)
`de01f188f2be7831f619ca09882cf2f55b0c9720` for the parser.
- Python bindings written against CPython's stable ABI, so one `cp310-abi3` wheel serves every CPython from 3.10.
- Node bindings written against Node-API, so one build runs on every Node.js that carries it.
- C# bindings written against the library's flat C ABI, so one assembly serves .NET Framework 2.0 and
modern .NET.

## Install

The library is built and installed with CMake:

```sh
cmake -S . -B build
cmake --build build
cmake --install build
```

A consumer finds the package with `find_package` and links the target it needs:

```cmake
find_package(pppp CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE pppp::fosu)
```

## Usage

```cpp
#include <pppp/beatmap.h>
#include <pppp/pppp.h>

pppp::Beatmap beatmap;
pppp::Status status = beatmap.load_file("map.osu");    // or beatmap.load_buffer(data, size)
if (!status.ok()) { puts(status.message()); return 1; }

pppp::Mods mods;
if (!mods.parse("HDDT").ok()) return 1;                 // "DT:speed_change=1.4" for settings
// pppp::Mods::from_legacy(pppp::mods::LEGACY_MOD_HIDDEN) for osu!stable's bitmask
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
```

## Python

```python
import pppp

beatmap = pppp.Beatmap.from_file("map.osu")

difficulty = pppp.Difficulty(mods="HD,DT").calculate(beatmap)
performance = pppp.Performance(mods="HD,DT", max_combo=789, accuracy=0.992, misses=2).calculate(beatmap, difficulty)

print(difficulty.star_rating, performance.total)
```

## C#

```csharp
using Pppp;

using (Beatmap beatmap = Beatmap.FromFile("map.osu")) {
    DifficultyAttributes difficulty = new Difficulty { Mods = "HD,DT" }.Calculate(beatmap);
    PerformanceAttributes performance =
        new Performance { Mods = "HD,DT", MaxCombo = 789, Accuracy = 0.992, Misses = 2 }.Calculate(beatmap, difficulty);

    Console.WriteLine(difficulty.StarRating + " " + performance.Total);
}
```

## Node

```js
import { Beatmap, Difficulty, Performance } from "@7mochi/pppp";

const beatmap = await Beatmap.fromFile("map.osu");

const difficulty = new Difficulty({ mods: "HD,DT" }).calculate(beatmap);
const performance = new Performance({ mods: "HD,DT", maxCombo: 789, accuracy: 0.992, misses: 2 }).calculate(beatmap, difficulty);

console.log(difficulty.starRating, performance.total);
```

## Options

| CMake option | Default | What it does |
| --- | --- | --- |
| `PPPP_WITH_FOSU` | ON | the parser adapter and the vendored parser |
| `PPPP_BUILD_TESTS` | top-level only | the suites (fetches doctest) |
| `PPPP_BUILD_PYTHON` | OFF | the Python extension module |
| `PPPP_BUILD_NODE` | OFF | the Node extension module |
| `PPPP_BUILD_CAPI` | OFF | the flat C ABI the C# bindings use |
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
