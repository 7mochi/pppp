/* The C++ install test program. When the parser is installed, it loads a beatmap from a string
   and checks the difficulty result. Without the parser it only checks that the headers compile and
   the library links. */
#include <cstdio>
#include <pppp/beatmaps/beatmap.h>
#include <pppp/pppp.h>
// The install test defines this when the parser package is installed.
#ifdef PPPP_INSTALL_TEST_WITH_FOSU
#include <pppp/beatmap.h>

static const char map_text[] = "osu file format v14\n"
                               "\n"
                               "[General]\n"
                               "Mode: 0\n"
                               "\n"
                               "[Difficulty]\n"
                               "CircleSize: 4\n"
                               "OverallDifficulty: 5\n"
                               "ApproachRate: 5\n"
                               "SliderMultiplier: 1.4\n"
                               "SliderTickRate: 1\n"
                               "\n"
                               "[TimingPoints]\n"
                               "1000,500,4,2,1,60,1,0\n"
                               "\n"
                               "[HitObjects]\n"
                               "256,192,1000,1,0,0:0:0:0:\n"
                               "256,192,1500,1,0,0:0:0:0:\n";
#endif

int main() {
#ifdef PPPP_INSTALL_TEST_WITH_FOSU
    pppp::Beatmap map;
    const pppp::Status load = map.load_buffer(map_text, sizeof(map_text) - 1);
    if (!load.ok()) {
        std::printf("load: %s\n", load.message());
        return 1;
    }
#else
    pppp::beatmaps::Beatmap map;
#endif

    pppp::Difficulty difficulty;
    pppp::DifficultyAttributes attributes;
    const pppp::Status status = difficulty.calculate(map, attributes);
    if (!status.ok()) {
        std::printf("difficulty: %s\n", status.message());
        return 2;
    }

#ifdef PPPP_INSTALL_TEST_WITH_FOSU
    if (attributes.max_combo() != 2 || attributes.star_rating() <= 0.0) {
        std::printf("attributes: max_combo=%d star_rating=%f\n", attributes.max_combo(),
                    attributes.star_rating());
        return 3;
    }
#endif

    std::printf("ok\n");
    return 0;
}
