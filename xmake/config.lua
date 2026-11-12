-- Shared settings for the files under xmake/. The options are declared in xmake.lua and read here.
pppp = {
    with_fosu = get_config("pppp_with_fosu") ~= false,
    build_tests = get_config("pppp_build_tests") ~= false,
    warnings_as_errors = get_config("pppp_warnings_as_errors") == true,
    doctest_dir = get_config("doctest_dir") or "",
    cjson_dir = get_config("cjson_dir") or "",
    isa = get_config("pppp_isa") or "auto",
}

if pppp.isa == "avx2" and not (is_arch("x86_64") or is_arch("x86")) then
    raise("isa=avx2 needs an x86 target")
end

if pppp.isa == "neon" and not is_arch("arm64", "aarch64") then
    raise("isa=neon needs an AArch64 target")
end

pppp.engine_suffix = is_plat("windows") and ".dll" or is_plat("macosx") and ".dylib" or ".so"
pppp.x86_engine = (pppp.isa == "auto" or pppp.isa == "avx2") and (is_arch("x86_64") or is_arch("x86"))
pppp.arm_engine = (pppp.isa == "auto" or pppp.isa == "neon") and is_arch("arm64", "aarch64")

function pppp.as_list(value)
    if type(value) == "table" then
        return value
    end
    return { value }
end

-- Join paths with the project root because included files run from their own directory.
function pppp.path(rel)
    return path.join(os.projectdir(), rel)
end

if pppp.build_tests and pppp.doctest_dir == "" then
    add_requires("pppp-doctest")
end

if pppp.build_tests and pppp.with_fosu and pppp.cjson_dir == "" then
    -- cJSON is only for the tests. Build it without LTO because the release test binaries cannot
    -- link the LTO version.
    add_requires("pppp-cjson", { configs = { lto = false } })
end
