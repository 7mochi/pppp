/* The C install test program. It includes the public header as plain C, loads a beatmap from a
   string, runs the osu! difficulty calculator, and checks the version. A broken header, a missing
   pppp_c package, or a missing parser engine makes the test fail. */
#include <pppp/capi.h>
#include <stdio.h>

static const char* map_text =
    "osu file format v14\n"
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

int main(void) {
    PpppNativeBeatmap beatmap;
    PpppNativeRuleset ruleset;
    PpppNativeOsuDifficultyCalculator calculator;
    PpppOsuDifficultyAttributes attributes;

    if (Pppp_Version() == 0) {
        return 1;
    }

    if (Pppp_VersionNumber() != PPPP_VERSION_NUMBER) {
        return 2;
    }

    if (Pppp_Beatmap_CreateFromText(map_text, &beatmap) != Pppp_Success) {
        return 3;
    }

    if (Pppp_Ruleset_CreateFromId(0, &ruleset) != Pppp_Success) {
        Pppp_Beatmap_Destroy(beatmap.handle);
        return 4;
    }

    if (Pppp_OsuDifficultyCalculator_Create(ruleset.handle, beatmap.handle, &calculator) != Pppp_Success) {
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(beatmap.handle);
        return 5;
    }

    if (Pppp_OsuDifficultyCalculator_Calculate(calculator.handle, 0, &attributes) != Pppp_Success) {
        Pppp_OsuDifficultyCalculator_Destroy(calculator.handle);
        Pppp_Ruleset_Destroy(ruleset.handle);
        Pppp_Beatmap_Destroy(beatmap.handle);
        return 6;
    }

    Pppp_OsuDifficultyCalculator_Destroy(calculator.handle);
    Pppp_Ruleset_Destroy(ruleset.handle);
    Pppp_Beatmap_Destroy(beatmap.handle);

    if (Pppp_ErrorHandler_GetLastMessage() != 0) {
        return 7;
    }

    puts(Pppp_Version());
    return 0;
}
