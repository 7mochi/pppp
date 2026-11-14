-- Tests are split into two groups: library and parser. xmake finds them and applies the timeouts.
-- Parser tests are built without SIMD and run on the scalar path.
if pppp.build_tests then
target("pppp_test_main")
    set_kind("object")
    set_default(false)

    add_files(pppp.path("tests/main.cpp"))

    pppp_doctest_flags()
    pppp_test_flags()
target_end()

if pppp.with_fosu then
target("pppp_cjson")
    set_default(false)

    if pppp.cjson_dir ~= "" then
        set_kind("static")

        add_files(path.join(pppp.cjson_dir, "cJSON.c"))
        add_sysincludedirs(pppp.cjson_dir, { public = true })
    else
        set_kind("headeronly")
        add_packages("pppp-cjson")
    end
target_end()
end
end

local function pppp_test(name, deps, scalar, setup)
    target("test_" .. name)
        set_kind("binary")
        set_default(false)

        add_files(pppp.path("tests/test_" .. name .. ".cpp"))
        add_deps("pppp_test_main", "pppp_internal")

        local uses_parser = false

        for _, dep in ipairs(pppp.as_list(deps)) do
            uses_parser = uses_parser or dep == "fosu"
        end

        pppp_doctest_flags()
        pppp_test_flags(uses_parser)

        for _, dep in ipairs(pppp.as_list(deps)) do
            add_deps(dep)
        end

        add_defines('PPPP_TEST_RESOURCES="' .. pppp.path("tests/resources") .. '"',
                    'PPPP_TEST_PARITY="' .. pppp.path("tests/parity/parity.json") .. '"')

        if setup then
            setup()
        end

        add_tests("default", { run_timeout = 300000, group = "library" })

        if scalar then
            add_tests("scalar", { run_timeout = 300000, group = "library",
                                  runenvs = { FOSU_BACKEND = "scalar" } })
        end
    target_end()
end

if pppp.build_tests then
    pppp_test("linq", "pppp")

    if pppp.with_fosu then
        pppp_test("beatmap", "fosu")
        pppp_test("timed_and_strains", "fosu")
        pppp_test("mod_behaviour", "fosu")
        pppp_test("backend", "fosu", true)
        pppp_test("parity_decode", {"fosu", "pppp_cjson"}, true, pppp_cjson_flags)
        pppp_test("parity_difficulty", {"fosu", "pppp_cjson"}, true, pppp_cjson_flags)
        pppp_test("parity_performance", {"fosu", "pppp_cjson"}, true, pppp_cjson_flags)
    end
end

if pppp.build_tests and pppp.with_fosu then
local fosu_tests = {
    "decimal", "sections", "hitobject_fields", "storage", "hardening", "numeric",
    "string_lookup", "mods", "stacking", "slider_paths", "slider_timing", "clients", "dispatch",
}

for _, name in ipairs(fosu_tests) do
    target("fosu_test_" .. name)
        set_kind("binary")
        set_default(false)

        add_files(pppp.path("third_party/fosu/tests/test_" .. name .. ".cpp"))
        add_includedirs(pppp.path("third_party/fosu/src"))
        add_defines("FOSU_DISABLE_SIMD")
        add_deps("fosu_parser", "pppp_test_main")

        pppp_doctest_flags()
        pppp_parser_test_flags()

        -- Windows uses Win32 threads instead of pthreads.
        if name == "dispatch" and not is_plat("windows") then
            add_syslinks("pthread")
        end

        add_tests("default", { run_timeout = 300000, group = "parser",
                              runenvs = { FOSU_BACKEND = "scalar" } })
    target_end()
end

pppp_test("capi", {"pppp_c", "pppp_cjson"}, false, pppp_cjson_flags)

target("test_capi_c")
    set_kind("binary")
    set_default(false)
    set_languages("c89")

    add_files(pppp.path("tests/test_capi_c.c"))
    add_includedirs(pppp.path("include"))
    add_deps("pppp_c")
    add_links("pppp_c")

    add_tests("default", { run_timeout = 300000, group = "library" })
target_end()
end
