#include "pppp/capi.h"
#include "pppp/backend.h"
#include "pppp/beatmap.h"
#include "pppp/difficulty.h"
#include "pppp/mods/mods.h"
#include <cstring>
#include <new>

struct PpppBeatmap {
    pppp::Beatmap map;
    unsigned refs;
};

struct PpppRuleset {
    int id;
};

struct PpppMods {
    pppp::mods::Mods mods;
};

struct PpppDifficultyCalculator {
    PpppBeatmap* beatmap;
    pppp::Difficulty difficulty;
    int ruleset_id;
};

struct PpppOsuDifficultyCalculator : PpppDifficultyCalculator {};
struct PpppTaikoDifficultyCalculator : PpppDifficultyCalculator {};
struct PpppCatchDifficultyCalculator : PpppDifficultyCalculator {};
struct PpppManiaDifficultyCalculator : PpppDifficultyCalculator {};

namespace {
#if defined(_MSC_VER)
    __declspec(thread) char g_last_message[256];
    __declspec(thread) PpppBool g_has_message;
#else
    __thread char g_last_message[256];
    __thread PpppBool g_has_message;
#endif

    void clear_error() {
        g_last_message[0] = '\0';
        g_has_message = 0;
    }

    void set_error(const char* message) {
        if (!message) {
            message = "unknown error";
        }

        size_t length = std::strlen(message);
        if (length >= sizeof(g_last_message)) {
            length = sizeof(g_last_message) - 1;
        }

        std::memcpy(g_last_message, message, length);
        g_last_message[length] = '\0';
        g_has_message = 1;
    }

    PpppErrorCode to_error_code(pppp::StatusCode::Value code) {
        switch (code) {
        case pppp::StatusCode::OK: return Pppp_Success;
        case pppp::StatusCode::INVALID_ARGUMENT: return Pppp_InvalidArgument;
        case pppp::StatusCode::ALLOCATION: return Pppp_Allocation;
        case pppp::StatusCode::PARSE: return Pppp_Parse;
        case pppp::StatusCode::NO_SLIDER_PATH_BACKEND: return Pppp_NoSliderPathBackend;
        case pppp::StatusCode::SLIDER_PATH: return Pppp_SliderPath;
        case pppp::StatusCode::INTERNAL: return Pppp_Internal;
        }

        return Pppp_Failure;
    }

    PpppErrorCode status_error(const pppp::Status& status) {
        if (status.ok()) {
            return Pppp_Success;
        }

        set_error(status.message());

        return to_error_code(status.code());
    }

    PpppErrorCode get_text(const char* data, size_t length, char* buffer, PpppInt32* size) {
        const PpppInt32 required = static_cast<PpppInt32>(length + 1);

        if (!buffer) {
            *size = required;

            return Pppp_BufferSizeQuery;
        }

        if (*size < 1) {
            set_error("the buffer size must be positive");

            return Pppp_InvalidArgument;
        }

        PpppInt32 copy = static_cast<PpppInt32>(length);
        if (copy > *size - 1) {
            copy = *size - 1;
        }

        if (copy > 0) {
            std::memcpy(buffer, data, static_cast<size_t>(copy));
        }
        buffer[copy] = '\0';

        return Pppp_Success;
    }

    PpppErrorCode get_vector_text(const std::vector<char>& text, char* buffer, PpppInt32* size) {
        if (text.empty()) {
            return get_text("", 0, buffer, size);
        }

        return get_text(&text[0], text.size() - 1, buffer, size);
    }

    void retain(PpppBeatmap* beatmap) { beatmap->refs++; }

    void release(PpppBeatmap* beatmap) {
        if (!beatmap) {
            return;
        }

        if (--beatmap->refs == 0) {
            delete beatmap;
        }
    }

    void fill_native(PpppNativeBeatmap* out, PpppBeatmap* handle) {
        const pppp::beatmaps::Beatmap& map = handle->map;

        out->handle = handle;
        out->ruleset_id = map.mode;
        out->beatmap_id = map.beatmap_id;
        out->approach_rate = map.difficulty.approach_rate;
        out->drain_rate = map.difficulty.drain_rate;
        out->overall_difficulty = map.difficulty.overall_difficulty;
        out->circle_size = map.difficulty.circle_size;
        out->slider_multiplier = map.difficulty.slider_multiplier;
        out->slider_tick_rate = map.difficulty.slider_tick_rate;
        out->format_version = map.format_version;
        out->stack_leniency = map.stack_leniency;
    }

    const char* ruleset_short_name(int id) {
        switch (id) {
        case pppp::Ruleset::OSU: return "osu";
        case pppp::Ruleset::TAIKO: return "taiko";
        case pppp::Ruleset::CATCH: return "fruits";
        case pppp::Ruleset::MANIA: return "mania";
        }

        return 0;
    }

    int ruleset_id_from_short_name(const char* name) {
        if (std::strcmp(name, "osu") == 0) {
            return pppp::Ruleset::OSU;
        }

        if (std::strcmp(name, "taiko") == 0) {
            return pppp::Ruleset::TAIKO;
        }

        if (std::strcmp(name, "fruits") == 0) {
            return pppp::Ruleset::CATCH;
        }

        if (std::strcmp(name, "mania") == 0) {
            return pppp::Ruleset::MANIA;
        }

        return -1;
    }

    void set_calculator_clock_rate(PpppDifficultyCalculator* calculator, double clock_rate) {
        calculator->difficulty = pppp::Difficulty()
                                     .ruleset(static_cast<pppp::Ruleset::Value>(calculator->ruleset_id))
                                     .clock_rate(clock_rate);
    }

    void clear_calculator_clock_rate(PpppDifficultyCalculator* calculator) {
        calculator->difficulty =
            pppp::Difficulty().ruleset(static_cast<pppp::Ruleset::Value>(calculator->ruleset_id));
    }

    PpppErrorCode calculate_into(const PpppDifficultyCalculator* calculator, PpppMods* mods,
                                 pppp::DifficultyAttributes& out) {
        pppp::Difficulty difficulty = calculator->difficulty;

        if (mods) {
            difficulty.mods(mods->mods);
        }

        return status_error(difficulty.calculate(calculator->beatmap->map, out));
    }

    template <class Calculator, class Native>
    PpppErrorCode create_calculator(int expected_ruleset, PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                    Native* out) {
        clear_error();

        if (!ruleset || !beatmap || !out) {
            set_error("a ruleset, a beatmap and an output are required");

            return Pppp_InvalidArgument;
        }

        if (ruleset->id != expected_ruleset) {
            set_error("the ruleset doesn't match the calculator");

            return Pppp_UnexpectedRuleset;
        }

        Calculator* calculator = new (std::nothrow) Calculator;
        if (!calculator) {
            set_error("out of memory");

            return Pppp_Allocation;
        }

        calculator->beatmap = beatmap;
        calculator->ruleset_id = expected_ruleset;
        calculator->difficulty =
            pppp::Difficulty().ruleset(static_cast<pppp::Ruleset::Value>(expected_ruleset));
        retain(beatmap);
        out->handle = calculator;

        return Pppp_Success;
    }

    template <class Calculator>
    void destroy_calculator(Calculator* calculator) {
        if (!calculator) {
            return;
        }

        release(calculator->beatmap);
        delete calculator;
    }

    void fill_osu(PpppOsuDifficultyAttributes* out, const pppp::OsuDifficultyAttributes& attributes) {
        out->star_rating = attributes.star_rating;
        out->max_combo = attributes.max_combo;
        out->aim_difficulty = attributes.aim_difficulty;
        out->speed_difficulty = attributes.speed_difficulty;
        out->reading_difficulty = attributes.reading_difficulty;
        out->flashlight_difficulty = attributes.flashlight_difficulty;
        out->slider_factor = attributes.slider_factor;
        out->aim_difficult_strain_count = attributes.aim_difficult_strain_count;
        out->speed_difficult_strain_count = attributes.speed_difficult_strain_count;
        out->reading_difficult_note_count = attributes.reading_difficult_note_count;
        out->aim_difficult_slider_count = attributes.aim_difficult_slider_count;
        out->aim_top_weighted_slider_factor = attributes.aim_top_weighted_slider_factor;
        out->speed_top_weighted_slider_factor = attributes.speed_top_weighted_slider_factor;
        out->speed_note_count = attributes.speed_note_count;
        out->hit_circle_count = attributes.hit_circle_count;
        out->slider_count = attributes.slider_count;
        out->large_tick_count = attributes.large_tick_count;
        out->spinner_count = attributes.spinner_count;
        out->nested_score_per_object = attributes.nested_score_per_object;
        out->legacy_score_base_multiplier = attributes.legacy_score_base_multiplier;
        out->maximum_legacy_combo_score = attributes.maximum_legacy_combo_score;
    }

    void fill_taiko(PpppTaikoDifficultyAttributes* out, const pppp::TaikoDifficultyAttributes& attributes) {
        out->star_rating = attributes.star_rating;
        out->max_combo = attributes.max_combo;
        out->mechanical_difficulty = attributes.mechanical_difficulty;
        out->rhythm_difficulty = attributes.rhythm_difficulty;
        out->reading_difficulty = attributes.reading_difficulty;
        out->colour_difficulty = attributes.colour_difficulty;
        out->stamina_difficulty = attributes.stamina_difficulty;
        out->mono_stamina_factor = attributes.mono_stamina_factor;
        out->consistency_factor = attributes.consistency_factor;
        out->stamina_top_strains = attributes.stamina_top_strains;
    }

    void fill_catch(PpppCatchDifficultyAttributes* out, const pppp::CatchDifficultyAttributes& attributes) {
        out->star_rating = attributes.star_rating;
        out->max_combo = attributes.max_combo;
    }

    void fill_mania(PpppManiaDifficultyAttributes* out, const pppp::ManiaDifficultyAttributes& attributes) {
        out->star_rating = attributes.star_rating;
        out->max_combo = attributes.max_combo;
    }
} // namespace

const char* Pppp_Version(void) { return PPPP_VERSION_STRING; }

PpppUint32 Pppp_VersionNumber(void) { return PPPP_VERSION_NUMBER; }

const char* Pppp_Backend(void) {
    switch (pppp::backend()) {
    case pppp::EngineKind::SCALAR: return "scalar";
    case pppp::EngineKind::AVX2: return "avx2";
    case pppp::EngineKind::NEON: return "neon";
    }

    return "scalar";
}

const char* Pppp_ErrorHandler_GetLastMessage(void) { return g_has_message ? g_last_message : 0; }

PpppErrorCode Pppp_Beatmap_CreateFromFile(const char* path, PpppNativeBeatmap* out) {
    clear_error();

    if (!path || !out) {
        set_error("a path and an output are required");
        return Pppp_InvalidArgument;
    }

    PpppBeatmap* beatmap = new (std::nothrow) PpppBeatmap;
    if (!beatmap) {
        set_error("out of memory");
        return Pppp_Allocation;
    }

    const pppp::Status status = beatmap->map.load_file(path);
    if (!status.ok()) {
        const PpppErrorCode code = status_error(status);
        delete beatmap;
        return code;
    }

    beatmap->refs = 0;
    retain(beatmap);
    fill_native(out, beatmap);

    return Pppp_Success;
}

PpppErrorCode Pppp_Beatmap_CreateFromText(const char* text, PpppNativeBeatmap* out) {
    clear_error();

    if (!text || !out) {
        set_error("a text and an output are required");

        return Pppp_InvalidArgument;
    }

    PpppBeatmap* beatmap = new (std::nothrow) PpppBeatmap;
    if (!beatmap) {
        set_error("out of memory");

        return Pppp_Allocation;
    }

    const pppp::Status status = beatmap->map.load_buffer(text, std::strlen(text));
    if (!status.ok()) {
        const PpppErrorCode code = status_error(status);
        delete beatmap;

        return code;
    }

    beatmap->refs = 0;
    retain(beatmap);
    fill_native(out, beatmap);

    return Pppp_Success;
}

PpppErrorCode Pppp_Beatmap_CreateFromBytes(const void* data, size_t size, PpppNativeBeatmap* out) {
    clear_error();

    if (!data || !out) {
        set_error("data and an output are required");

        return Pppp_InvalidArgument;
    }

    PpppBeatmap* beatmap = new (std::nothrow) PpppBeatmap;
    if (!beatmap) {
        set_error("out of memory");

        return Pppp_Allocation;
    }

    const pppp::Status status = beatmap->map.load_buffer(data, size);
    if (!status.ok()) {
        const PpppErrorCode code = status_error(status);
        delete beatmap;

        return code;
    }

    beatmap->refs = 0;
    retain(beatmap);
    fill_native(out, beatmap);

    return Pppp_Success;
}

PpppErrorCode Pppp_Beatmap_GetTitle(PpppBeatmap* beatmap, char* buffer, PpppInt32* size) {
    clear_error();

    if (!beatmap || !size) {
        set_error("a beatmap and a size are required");
        return Pppp_InvalidArgument;
    }

    return get_vector_text(beatmap->map.title, buffer, size);
}

PpppErrorCode Pppp_Beatmap_GetArtist(PpppBeatmap* beatmap, char* buffer, PpppInt32* size) {
    clear_error();

    if (!beatmap || !size) {
        set_error("a beatmap and a size are required");
        return Pppp_InvalidArgument;
    }

    return get_vector_text(beatmap->map.artist, buffer, size);
}

PpppErrorCode Pppp_Beatmap_GetVersion(PpppBeatmap* beatmap, char* buffer, PpppInt32* size) {
    clear_error();

    if (!beatmap || !size) {
        set_error("a beatmap and a size are required");
        return Pppp_InvalidArgument;
    }

    return get_vector_text(beatmap->map.version, buffer, size);
}

void Pppp_Beatmap_Destroy(PpppBeatmap* beatmap) { release(beatmap); }

PpppErrorCode Pppp_Ruleset_CreateFromId(PpppInt32 id, PpppNativeRuleset* out) {
    clear_error();

    if (!out) {
        set_error("an output is required");

        return Pppp_InvalidArgument;
    }

    if (!ruleset_short_name(id)) {
        set_error("unknown ruleset id");

        return Pppp_RulesetUnavailable;
    }

    PpppRuleset* ruleset = new (std::nothrow) PpppRuleset;
    if (!ruleset) {
        set_error("out of memory");

        return Pppp_Allocation;
    }

    ruleset->id = id;
    out->handle = ruleset;
    out->ruleset_id = id;

    return Pppp_Success;
}

PpppErrorCode Pppp_Ruleset_CreateFromShortName(const char* name, PpppNativeRuleset* out) {
    clear_error();

    if (!name || !out) {
        set_error("a short name and an output are required");

        return Pppp_InvalidArgument;
    }

    const int id = ruleset_id_from_short_name(name);
    if (id < 0) {
        set_error("unknown ruleset short name");

        return Pppp_RulesetUnavailable;
    }

    PpppRuleset* ruleset = new (std::nothrow) PpppRuleset;
    if (!ruleset) {
        set_error("out of memory");

        return Pppp_Allocation;
    }

    ruleset->id = id;
    out->handle = ruleset;
    out->ruleset_id = id;

    return Pppp_Success;
}

PpppErrorCode Pppp_Ruleset_GetShortName(PpppRuleset* ruleset, char* buffer, PpppInt32* size) {
    clear_error();

    if (!ruleset || !size) {
        set_error("a ruleset and a size are required");

        return Pppp_InvalidArgument;
    }

    const char* name = ruleset_short_name(ruleset->id);
    if (!name) {
        set_error("unknown ruleset id");

        return Pppp_Internal;
    }

    return get_text(name, std::strlen(name), buffer, size);
}

void Pppp_Ruleset_Destroy(PpppRuleset* ruleset) { delete ruleset; }

PpppErrorCode Pppp_Mods_Create(PpppNativeMods* out) {
    clear_error();

    if (!out) {
        set_error("an output is required");

        return Pppp_InvalidArgument;
    }

    PpppMods* mods = new (std::nothrow) PpppMods;
    if (!mods) {
        set_error("out of memory");

        return Pppp_Allocation;
    }

    out->handle = mods;

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_Parse(PpppMods* mods, const char* specification) {
    clear_error();

    if (!mods || !specification) {
        set_error("mods and a specification are required");

        return Pppp_InvalidArgument;
    }

    const pppp::Status status = mods->mods.parse(specification);
    if (!status.ok()) {
        set_error(status.message());

        return Pppp_InvalidArgument;
    }

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_FromLegacy(PpppMods* mods, PpppUint32 bits, PpppInt32 ruleset) {
    clear_error();

    if (!mods) {
        set_error("mods are required");

        return Pppp_InvalidArgument;
    }

    if (!ruleset_short_name(ruleset)) {
        set_error("unknown ruleset id");
        return Pppp_InvalidArgument;
    }

    mods->mods = pppp::mods::Mods::from_legacy(bits, ruleset);

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_AddClassic(PpppMods* mods) {
    clear_error();

    if (!mods) {
        set_error("mods are required");

        return Pppp_InvalidArgument;
    }

    mods->mods.add_classic();

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_Contains(PpppMods* mods, const char* acronym, PpppBool* out) {
    clear_error();

    if (!mods || !acronym || !out) {
        set_error("mods, an acronym and an output are required");

        return Pppp_InvalidArgument;
    }

    pppp::mods::Mod mod;
    if (pppp::mods::mod_from_acronym(&mod, acronym) != 0) {
        set_error("unknown mod acronym");

        return Pppp_InvalidArgument;
    }

    *out = mods->mods.contains(mod.id) ? 1 : 0;

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_GetCount(PpppMods* mods, PpppInt32* out) {
    clear_error();

    if (!mods || !out) {
        set_error("mods and an output are required");

        return Pppp_InvalidArgument;
    }

    *out = static_cast<PpppInt32>(mods->mods.size());

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_GetAcronym(PpppMods* mods, PpppInt32 index, char* buffer, PpppInt32* size) {
    clear_error();

    if (!mods || !size) {
        set_error("mods and a size are required");

        return Pppp_InvalidArgument;
    }

    if (index < 0 || static_cast<size_t>(index) >= mods->mods.size()) {
        set_error("the mod index is out of range");

        return Pppp_InvalidArgument;
    }

    const char* acronym = pppp::mods::mod_acronym(mods->mods[static_cast<size_t>(index)].id);
    if (!acronym) {
        set_error("the mod has no acronym");

        return Pppp_Internal;
    }

    return get_text(acronym, std::strlen(acronym), buffer, size);
}

PpppErrorCode Pppp_Mods_GetClockRate(PpppMods* mods, double* out) {
    clear_error();

    if (!mods || !out) {
        set_error("mods and an output are required");

        return Pppp_InvalidArgument;
    }

    *out = mods->mods.clock_rate();

    return Pppp_Success;
}

PpppErrorCode Pppp_Mods_GetLegacyBits(PpppMods* mods, PpppInt32 ruleset, PpppUint32* out) {
    clear_error();

    if (!mods || !out) {
        set_error("mods and an output are required");

        return Pppp_InvalidArgument;
    }

    if (!ruleset_short_name(ruleset)) {
        set_error("unknown ruleset id");
        return Pppp_InvalidArgument;
    }

    *out = mods->mods.to_legacy(ruleset);

    return Pppp_Success;
}

void Pppp_Mods_Destroy(PpppMods* mods) { delete mods; }

PpppErrorCode Pppp_OsuDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                  PpppNativeOsuDifficultyCalculator* out) {
    return create_calculator<PpppOsuDifficultyCalculator>(0, ruleset, beatmap, out);
}

PpppErrorCode Pppp_OsuDifficultyCalculator_Calculate(PpppOsuDifficultyCalculator* calculator, PpppMods* mods,
                                                     PpppOsuDifficultyAttributes* out) {
    clear_error();

    if (!calculator || !out) {
        set_error("a calculator and an output are required");

        return Pppp_InvalidArgument;
    }

    pppp::DifficultyAttributes attributes;
    const PpppErrorCode code = calculate_into(calculator, mods, attributes);
    if (code != Pppp_Success) {
        return code;
    }

    const pppp::OsuDifficultyAttributes* value = attributes.osu();
    if (!value) {
        set_error("the calculator did not produce osu! attributes");

        return Pppp_Internal;
    }

    fill_osu(out, *value);

    return Pppp_Success;
}

PpppErrorCode Pppp_OsuDifficultyCalculator_SetClockRate(PpppOsuDifficultyCalculator* calculator,
                                                        double clock_rate) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    set_calculator_clock_rate(calculator, clock_rate);

    return Pppp_Success;
}

PpppErrorCode Pppp_OsuDifficultyCalculator_ClearClockRate(PpppOsuDifficultyCalculator* calculator) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    clear_calculator_clock_rate(calculator);

    return Pppp_Success;
}

void Pppp_OsuDifficultyCalculator_Destroy(PpppOsuDifficultyCalculator* calculator) {
    destroy_calculator(calculator);
}

PpppErrorCode Pppp_TaikoDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                    PpppNativeTaikoDifficultyCalculator* out) {
    return create_calculator<PpppTaikoDifficultyCalculator>(1, ruleset, beatmap, out);
}

PpppErrorCode Pppp_TaikoDifficultyCalculator_Calculate(PpppTaikoDifficultyCalculator* calculator,
                                                       PpppMods* mods, PpppTaikoDifficultyAttributes* out) {
    clear_error();

    if (!calculator || !out) {
        set_error("a calculator and an output are required");

        return Pppp_InvalidArgument;
    }

    pppp::DifficultyAttributes attributes;
    const PpppErrorCode code = calculate_into(calculator, mods, attributes);
    if (code != Pppp_Success) {
        return code;
    }

    const pppp::TaikoDifficultyAttributes* value = attributes.taiko();
    if (!value) {
        set_error("the calculator did not produce taiko attributes");

        return Pppp_Internal;
    }

    fill_taiko(out, *value);

    return Pppp_Success;
}

PpppErrorCode Pppp_TaikoDifficultyCalculator_SetClockRate(PpppTaikoDifficultyCalculator* calculator,
                                                          double clock_rate) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    set_calculator_clock_rate(calculator, clock_rate);

    return Pppp_Success;
}

PpppErrorCode Pppp_TaikoDifficultyCalculator_ClearClockRate(PpppTaikoDifficultyCalculator* calculator) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    clear_calculator_clock_rate(calculator);

    return Pppp_Success;
}

void Pppp_TaikoDifficultyCalculator_Destroy(PpppTaikoDifficultyCalculator* calculator) {
    destroy_calculator(calculator);
}

PpppErrorCode Pppp_CatchDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                    PpppNativeCatchDifficultyCalculator* out) {
    return create_calculator<PpppCatchDifficultyCalculator>(2, ruleset, beatmap, out);
}

PpppErrorCode Pppp_CatchDifficultyCalculator_Calculate(PpppCatchDifficultyCalculator* calculator,
                                                       PpppMods* mods, PpppCatchDifficultyAttributes* out) {
    clear_error();

    if (!calculator || !out) {
        set_error("a calculator and an output are required");

        return Pppp_InvalidArgument;
    }

    pppp::DifficultyAttributes attributes;
    const PpppErrorCode code = calculate_into(calculator, mods, attributes);
    if (code != Pppp_Success) {
        return code;
    }

    const pppp::CatchDifficultyAttributes* value = attributes.fruits();
    if (!value) {
        set_error("the calculator did not produce osu!catch attributes");

        return Pppp_Internal;
    }

    fill_catch(out, *value);

    return Pppp_Success;
}

PpppErrorCode Pppp_CatchDifficultyCalculator_SetClockRate(PpppCatchDifficultyCalculator* calculator,
                                                          double clock_rate) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    set_calculator_clock_rate(calculator, clock_rate);

    return Pppp_Success;
}

PpppErrorCode Pppp_CatchDifficultyCalculator_ClearClockRate(PpppCatchDifficultyCalculator* calculator) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    clear_calculator_clock_rate(calculator);

    return Pppp_Success;
}

void Pppp_CatchDifficultyCalculator_Destroy(PpppCatchDifficultyCalculator* calculator) {
    destroy_calculator(calculator);
}

PpppErrorCode Pppp_ManiaDifficultyCalculator_Create(PpppRuleset* ruleset, PpppBeatmap* beatmap,
                                                    PpppNativeManiaDifficultyCalculator* out) {
    return create_calculator<PpppManiaDifficultyCalculator>(3, ruleset, beatmap, out);
}

PpppErrorCode Pppp_ManiaDifficultyCalculator_Calculate(PpppManiaDifficultyCalculator* calculator,
                                                       PpppMods* mods, PpppManiaDifficultyAttributes* out) {
    clear_error();

    if (!calculator || !out) {
        set_error("a calculator and an output are required");

        return Pppp_InvalidArgument;
    }

    pppp::DifficultyAttributes attributes;
    const PpppErrorCode code = calculate_into(calculator, mods, attributes);
    if (code != Pppp_Success) {
        return code;
    }

    const pppp::ManiaDifficultyAttributes* value = attributes.mania();
    if (!value) {
        set_error("the calculator did not produce osu!mania attributes");

        return Pppp_Internal;
    }

    fill_mania(out, *value);

    return Pppp_Success;
}

PpppErrorCode Pppp_ManiaDifficultyCalculator_SetClockRate(PpppManiaDifficultyCalculator* calculator,
                                                          double clock_rate) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    set_calculator_clock_rate(calculator, clock_rate);

    return Pppp_Success;
}

PpppErrorCode Pppp_ManiaDifficultyCalculator_ClearClockRate(PpppManiaDifficultyCalculator* calculator) {
    clear_error();

    if (!calculator) {
        set_error("a calculator is required");

        return Pppp_InvalidArgument;
    }

    clear_calculator_clock_rate(calculator);

    return Pppp_Success;
}

void Pppp_ManiaDifficultyCalculator_Destroy(PpppManiaDifficultyCalculator* calculator) {
    destroy_calculator(calculator);
}
