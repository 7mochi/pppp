#include "pppp/capi.h"
#include "pppp/backend.h"
#include "pppp/beatmap.h"
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
        case 0: return "osu";
        case 1: return "taiko";
        case 2: return "fruits";
        case 3: return "mania";
        }

        return 0;
    }

    int ruleset_id_from_short_name(const char* name) {
        if (std::strcmp(name, "osu") == 0) {
            return 0;
        }

        if (std::strcmp(name, "taiko") == 0) {
            return 1;
        }

        if (std::strcmp(name, "fruits") == 0) {
            return 2;
        }

        if (std::strcmp(name, "mania") == 0) {
            return 3;
        }

        return -1;
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
