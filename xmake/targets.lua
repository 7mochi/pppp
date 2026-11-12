-- Each target is defined after the targets it depends on. Wildcards pick up new source files.
target("pppp_options")
    set_kind("headeronly")

    add_includedirs(pppp.path("include"), { public = true })
    add_sysincludedirs(pppp.path("third_party/optional-bare/include"), pppp.path("third_party/variant-lite/include"), { public = true })
    add_headerfiles(pppp.path("include/(pppp/**.h)"))
target_end()

local function pppp_library(name, basename, files, deps, extra)
    target(name)
        set_kind("static")
        set_basename(basename)

        add_includedirs(pppp.path("src"))

        for _, file in ipairs(pppp.as_list(files)) do
            add_files(pppp.path(file))
        end

        add_deps("pppp_options")

        pppp_library_flags()

        for _, dep in ipairs(pppp.as_list(deps)) do
            add_deps(dep)
        end

        if extra then
            extra()
        end
    target_end()
end

target("utils")
    set_kind("static")
    set_basename("pppp_utils")

    add_includedirs(pppp.path("src"))
    add_files(pppp.path("src/pppp/utils/**.cpp"))

    add_deps("pppp_options")

    pppp_library_flags()

    after_install(function (target)
        local dir = path.join(target:installdir(), "include", "nonstd")
        os.mkdir(dir)
        os.cp(path.join(os.projectdir(), "third_party/optional-bare/include/nonstd/*.hpp"), dir)
        os.cp(path.join(os.projectdir(), "third_party/variant-lite/include/nonstd/*.hpp"), dir)
        os.cp(path.join(os.projectdir(), "third_party/optional-bare/LICENSE.txt"), path.join(dir, "optional-bare-LICENSE.txt"))
        os.cp(path.join(os.projectdir(), "third_party/variant-lite/LICENSE.txt"), path.join(dir, "variant-lite-LICENSE.txt"))
    end)
target_end()

pppp_library("beatmaps", "pppp_beatmaps", "src/pppp/beatmaps/**.cpp", "utils")
pppp_library("mods", "pppp_mods", "src/pppp/mods/**.cpp", "utils")
pppp_library("common", "pppp_common", "src/pppp/common/**.cpp", {"utils", "mods"})

-- osu!catch
pppp_library("catch_base", "pppp_catch_base",
    {"src/pppp/fruits/*.cpp", "src/pppp/fruits/mods/*.cpp", "src/pppp/fruits/object/*.cpp"},
    {"utils", "beatmaps", "common", "mods"})

pppp_library("catch_difficulty", "pppp_catch_difficulty",
    "src/pppp/fruits/difficulty/**.cpp", {"catch_base", "common"},
    function() remove_files(pppp.path("src/pppp/fruits/difficulty/catch_performance_*.cpp")) end)

pppp_library("catch_performance", "pppp_catch_performance",
    "src/pppp/fruits/difficulty/catch_performance_*.cpp", "catch_difficulty")

target("catch")
    set_kind("headeronly")
    add_deps("catch_performance")
target_end()

-- osu!mania
pppp_library("mania_base", "pppp_mania_base",
    {"src/pppp/mania/*.cpp", "src/pppp/mania/mods/*.cpp", "src/pppp/mania/patterns/*.cpp"},
    {"utils", "beatmaps", "common", "mods"})

pppp_library("mania_difficulty", "pppp_mania_difficulty",
    "src/pppp/mania/difficulty/**.cpp", {"mania_base", "common"},
    function() remove_files(pppp.path("src/pppp/mania/difficulty/mania_performance_*.cpp")) end)

pppp_library("mania_performance", "pppp_mania_performance",
    "src/pppp/mania/difficulty/mania_performance_*.cpp", "mania_difficulty")

target("mania")
    set_kind("headeronly")
    add_deps("mania_performance")
target_end()

-- osu!standard
pppp_library("osu_base", "pppp_osu_base",
    {"src/pppp/osu/*.cpp", "src/pppp/osu/mods/*.cpp", "src/pppp/osu/object/*.cpp"},
    {"utils", "beatmaps", "common", "mods"})

pppp_library("osu_difficulty", "pppp_osu_difficulty",
    "src/pppp/osu/difficulty/**.cpp", {"osu_base", "common"},
    function()
        remove_files(pppp.path("src/pppp/osu/difficulty/osu_performance_*.cpp"),
                     pppp.path("src/pppp/osu/difficulty/osu_legacy_score_miss_calculator.cpp"))
    end)

pppp_library("osu_performance", "pppp_osu_performance",
    "src/pppp/osu/difficulty/osu_performance_*.cpp", "osu_difficulty",
    function() add_files(pppp.path("src/pppp/osu/difficulty/osu_legacy_score_miss_calculator.cpp")) end)

target("osu")
    set_kind("headeronly")
    add_deps("osu_performance")
target_end()

-- osu!taiko
pppp_library("taiko_base", "pppp_taiko_base",
    {"src/pppp/taiko/*.cpp", "src/pppp/taiko/mods/*.cpp"},
    {"utils", "beatmaps", "common", "mods"})

pppp_library("taiko_difficulty", "pppp_taiko_difficulty",
    "src/pppp/taiko/difficulty/**.cpp", {"taiko_base", "common"},
    function() remove_files(pppp.path("src/pppp/taiko/difficulty/taiko_performance_*.cpp")) end)

pppp_library("taiko_performance", "pppp_taiko_performance",
    "src/pppp/taiko/difficulty/taiko_performance_*.cpp", "taiko_difficulty")

target("taiko")
    set_kind("headeronly")
    add_deps("taiko_performance")
target_end()

pppp_library("api", "pppp_api",
    {"src/pppp/difficulty.cpp", "src/pppp/performance.cpp"},
    {"catch", "mania", "osu", "taiko"},
    function() pppp_install_pkgconfig("pppp") end)

target("pppp")
    set_kind("headeronly")
    add_deps("api")
target_end()

target("pppp_internal")
    set_kind("headeronly")
    set_default(false)

    add_includedirs(pppp.path("src"), { public = true })
target_end()

if pppp.with_fosu then
target("fosu_parser")
    set_kind("static")
    set_basename("fosu")

    add_files(
        pppp.path("third_party/fosu/src/fosu/engine/parsing/decimal.cpp"),
        pppp.path("third_party/fosu/src/fosu/engine/parsing/lookup_tables.cpp"),
        pppp.path("third_party/fosu/src/fosu/engine/parse_document.cpp"),
        pppp.path("third_party/fosu/src/fosu/engine/runtime/loader.cc"),
        pppp.path("third_party/fosu/src/fosu/runtime.cc"))
    add_includedirs(pppp.path("third_party/fosu/src"), { public = true })
    add_sysincludedirs(pppp.path("third_party/optional-bare/include"), pppp.path("third_party/span-lite/include"),
                       pppp.path("third_party/string-view-lite/include"), pppp.path("third_party/atomic-lite/include"),
                       { public = true })
    add_defines('FOSU_DEFAULT_BACKEND="auto"', 'FOSU_ENGINE_SUFFIX="' .. pppp.engine_suffix .. '"',
                "FOSU_BUILDING_RUNTIME", "FOSU_DISABLE_SIMD")

    if not is_plat("windows") and (is_arch("x86_64") or is_arch("x86")) then
        add_cxflags("-march=x86-64")
    end

    if pppp.x86_engine then
        add_defines("FOSU_HAS_AVX2")
    elseif pppp.arm_engine then
        add_defines("FOSU_HAS_NEON")
    end

    pppp_library_flags(true)

    if is_plat("linux") then
        add_syslinks("dl")
    end

    add_headerfiles(pppp.path("third_party/fosu/src/(fosu/**.h)"))

    after_install(function (target)
        local dir = path.join(target:installdir(), "include", "nonstd")
        os.mkdir(dir)
        os.cp(path.join(os.projectdir(), "third_party/span-lite/include/nonstd/*.hpp"), dir)
        os.cp(path.join(os.projectdir(), "third_party/string-view-lite/include/nonstd/*.hpp"), dir)
        os.cp(path.join(os.projectdir(), "third_party/atomic-lite/include/nonstd/*.hpp"), dir)
        os.cp(path.join(os.projectdir(), "third_party/span-lite/LICENSE.txt"), path.join(dir, "span-lite-LICENSE.txt"))
        os.cp(path.join(os.projectdir(), "third_party/string-view-lite/LICENSE.txt"), path.join(dir, "string-view-lite-LICENSE.txt"))
        os.cp(path.join(os.projectdir(), "third_party/atomic-lite/LICENSE.txt"), path.join(dir, "atomic-lite-LICENSE.txt"))
    end)
target_end()

local function fosu_engine(variant, defines, flags)
    target("fosu_engine_" .. variant)
        set_kind("shared")

        add_files(
            pppp.path("third_party/fosu/src/fosu/engine/runtime/entry.cc"),
            pppp.path("third_party/fosu/src/fosu/engine/parse_document.cpp"),
            pppp.path("third_party/fosu/src/fosu/engine/parsing/decimal.cpp"),
            pppp.path("third_party/fosu/src/fosu/engine/parsing/lookup_tables.cpp"))
        add_includedirs(pppp.path("third_party/fosu/src"))
        add_sysincludedirs(pppp.path("third_party/optional-bare/include"), pppp.path("third_party/span-lite/include"),
                           pppp.path("third_party/string-view-lite/include"), pppp.path("third_party/atomic-lite/include"))
        add_defines(table.unpack(defines))

        if is_plat("windows") then
            if variant == "avx2" then
                add_cxflags("/arch:AVX2")
            end
        else
            add_cxflags(table.unpack(flags))
            add_ldflags("-Wl,--exclude-libs,ALL")
        end

        pppp_library_flags(true)
    target_end()
end

if pppp.x86_engine then
    fosu_engine("avx2", {"FOSU_COMPILE_AVX2", "FOSU_HAS_AVX2"},
                {"-march=x86-64-v3", "-fno-plt", "-fvisibility=hidden"})
elseif pppp.arm_engine then
    fosu_engine("neon", {"FOSU_HAS_NEON"}, {"-fvisibility=hidden"})
end

target("fosu")
    set_kind("static")
    set_basename("pppp_fosu")

    add_includedirs(pppp.path("src"))
    add_files(pppp.path("src/pppp/fosu/beatmap.cpp"), pppp.path("src/pppp/fosu/backend.cpp"))

    add_deps("pppp", "fosu_parser")

    pppp_library_flags(true)

    if pppp.x86_engine then
        add_deps("fosu_engine_avx2")
    elseif pppp.arm_engine then
        add_deps("fosu_engine_neon")
    end

    if is_plat("linux") then
        add_syslinks("dl")
    end

    pppp_install_pkgconfig("pppp_fosu")
target_end()

end
