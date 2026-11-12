#include <cstdio>
#include <pppp/beatmaps/beatmap.h>
#include <pppp/pppp.h>

int main() {
    pppp::beatmaps::Beatmap map;
    pppp::Difficulty difficulty;
    pppp::DifficultyAttributes attributes;
    const pppp::Status status = difficulty.calculate(map, attributes);

    std::printf("%d\n", status.ok() ? 1 : 0);
    return status.ok() ? 0 : 1;
}
