# PerformancePoints++

Calculate osu! difficulty and performance attributes for all four gamemodes in ISO C++98.

- All four gamemodes, with the mods each one supports.
- Ported from [osu!lazer](https://github.com/ppy/osu) and verified against its `2026.730.0` release; the
parser is from [fosu](https://github.com/cmyui/fast-osu-beatmap-parser)
`de01f188f2be7831f619ca09882cf2f55b0c9720`.
- Python bindings use the C ABI through generated `ctypes` code. One wheel per platform supports
CPython 3.10 and newer.
- Node bindings use the same C ABI through [Koffi](https://koffi.dev), so there's no addon to compile.
- C# bindings use the same C ABI. One assembly works on .NET Framework 2.0 and modern .NET.

## Install

Each binding is published to its own registry:

```sh
dotnet add package pppp-dotnet   # C#, .NET Framework 2.0 and modern .NET
pip install pppp-py              # Python 3.10 and newer
npm install @7mochi/pppp         # Node.js
```

Each release is published on the [releases page](https://github.com/7mochi/pppp/releases), one per
binding, with its package attached.

The library itself is built and installed with CMake:

```sh
cmake -S . -B build
cmake --build build
cmake --install build
```

A consumer finds the package with `find_package` and links the target it needs:

```cmake
find_package(pppp CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE pppp::fosu)
pppp_place_engine(your_target)   # puts the SIMD engine next to your_target
```

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

// The parser engine the loader picked, read-only: pppp::EngineKind::SCALAR, AVX2 or NEON.
// pppp::backend() == pppp::EngineKind::AVX2 on an x86-64 host, for instance.
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
| `PPPP_WITH_FOSU` | ON | the `.osu` parser and its adapter |
| `PPPP_ISA` | auto | parser engine: `auto`, `scalar`, `avx2` or `neon` |
| `PPPP_BUILD_TESTS` | top-level only | the test suites (fetches doctest) |
| `PPPP_BUILD_PYTHON` | OFF | the native library the Python package loads |
| `PPPP_BUILD_CAPI` | OFF | the C ABI shared library the Python, Node and C# bindings use |
| `PPPP_WIN2K` | OFF | the shim that keeps binaries loadable on Windows 2000 |
| `PPPP_INSTALL` | top-level only | the `find_package(pppp CONFIG)` package |
| `PPPP_WARNINGS_AS_ERRORS` | OFF | `-Werror` on the library's warnings |
| `PPPP_PGO` | OFF | profile-guided optimization: `GENERATE` instruments, `USE` consumes the profile (`make build-pgo` runs both, training on the parity suites) |
| `PPPP_PGO_DIR` | `<build>/../profile-data` | where the profile is written and read (GCC keeps it beside each object) |

## Development

The toolchain comes from the flake:

```sh
nix develop        # cmake, ninja, gcc, clang, python, node, .NET and pre-commit
make test-auto     # build and run the suites
make check-format  # the C++ formatting, checked
make build-pgo     # a PGO Release build in build_pgo (GCC or Clang)
```

Formatting, the Python types and the file hygiene are also enforced by
[pre-commit](https://pre-commit.com). The dev shell installs the hooks when you
enter it; to run them by hand:

```sh
pre-commit run --all-files
```

`clang-format` and `nixfmt` come from the dev shell; outside it the hooks fall
back to `nix develop`.

The Node binding is formatted and linted with [Biome](https://biomejs.dev), which
also sorts the imports. It comes from the dev shell, and as a dev dependency for
anyone running npm outside Nix:

```sh
npm run lint        # check and fix node/
npm run lint:check  # check only
```

Inside `nix develop`, run `biome` directly rather than `npm run lint`: npm puts
`node_modules/.bin` first on `PATH`, and the prebuilt Biome binary there doesn't
start on NixOS.

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
