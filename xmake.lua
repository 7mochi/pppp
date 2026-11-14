-- xmake only runs tasks defined in this root file. The rest of the build is in xmake/.
set_project("pppp")
set_version("1.0.0")
set_languages("c++98")
set_policy("build.warning", true)

-- Use Apple's compiler and sdk together on macos.
if is_plat("macosx") then
    set_toolchains("xcode")
end

if is_mode("debug") then
    set_symbols("debug")
elseif is_mode("release") then
    set_optimize("fastest")
    add_defines("NDEBUG")
    set_policy("build.optimization.lto", true)
    -- Programs that link with pkg-config don't use -flto. The release archives include both
    -- regular and LTO objects.
    add_cxflags("-ffat-lto-objects")
end

-- The test dependencies have a fixed URL and checksum. The *_dir options point the build at
-- local copies instead.
option("doctest_dir")
    set_default("")
    set_description("Use doctest.h from this directory instead of downloading doctest")
option_end()
option("cjson_dir")
    set_default("")
    set_description("Use cJSON.c and cJSON.h from this directory instead of downloading cJSON")
option_end()

option("pppp_with_fosu")
    set_default(true)
    set_showmenu(true)
    set_description("Build the .osu parser and the adapter that connects it to pppp")
option_end()
option("pppp_build_tests")
    set_default(true)
    set_showmenu(true)
    set_description("Build the test suites")
option_end()
option("pppp_warnings_as_errors")
    set_default(false)
    set_showmenu(true)
    set_description("Treat compiler warnings as errors")
option_end()
option("pppp_isa")
    set_default("auto")
    set_values("auto", "scalar", "avx2", "neon")
    set_description("Set the parser engine: auto, scalar, avx2 or neon")
option_end()

add_repositories("pppp-repo xmake")

includes("xmake/config.lua")
includes("xmake/flags.lua")
includes("xmake/install.lua")
includes("xmake/targets.lua")
includes("xmake/tests.lua")

-- Each header is compiled alone. A header that only compiles after another one is included
-- fails here.
task("check_headers")
    set_category("action")
    set_menu({
        usage = "xmake check_headers",
        description = "Compile every public header on its own",
    })
    on_run(function ()
        import("core.project.project")
        local target = project.target("pppp")
        local cxx = target:tool("cxx")
        local function p(rel) return path.join(os.projectdir(), rel) end
        local includes = {
            "-I" .. p("include"),
            "-isystem", p("third_party/optional-bare/include"),
            "-isystem", p("third_party/variant-lite/include"),
            "-isystem", p("third_party/span-lite/include"),
            "-isystem", p("third_party/string-view-lite/include"),
            "-isystem", p("third_party/atomic-lite/include"),
            "-isystem", p("third_party/fosu/src"),
        }

        for _, header in ipairs(os.files(p("include/pppp/**.h"))) do
            os.execv(cxx, table.join({"-std=c++98", "-fsyntax-only"}, includes, {header}))
        end
    end)
task_end()

-- Installs the library and builds a small program with pkg-config, like a user would.
task("test_install")
    set_category("action")
    set_menu({
        usage = "xmake test_install",
        description = "Install the library and build a small pkg-config program",
    })
    on_run(function ()
        import("core.project.project")
        local target = project.target("pppp")
        local cxx = target:tool("cxx")
        local cc = target:tool("cc")
        local function p(rel) return path.join(os.projectdir(), rel) end
        local function pkgflags(package)
            local out = {}
            local flags = (os.iorunv("pkg-config", {"--cflags", "--libs", package})):trim()
            for _, arg in ipairs(string.split(flags, " ", { plain = true })) do
                if arg ~= "" then
                    table.insert(out, arg)
                end
            end
            return out
        end
        local prefix = path.join(os.tmpdir(), "pppp-test-install")

        os.tryrm(prefix)
        os.execv("xmake", {"install", "-y", "-o", prefix})
        os.setenv("PKG_CONFIG_PATH", path.join(prefix, "lib", "pkgconfig"))

        -- The C++ consumer: the static pppp package, plus the parser when fosu is built.
        -- The install only has pppp_fosu.pc when the parser is built.
        local with_fosu = os.isfile(path.join(prefix, "lib", "pkgconfig", "pppp_fosu.pc"))
        local package = with_fosu and "pppp_fosu" or "pppp"
        local parser_define = with_fosu and {"-DPPPP_INSTALL_TEST_WITH_FOSU"} or {}
        local cxx_args = table.join({"-std=c++98"}, parser_define,
                                    {p("tests/install/consumer.cpp"), "-o",
                                     path.join(prefix, "consumer")}, pkgflags(package))
        os.execv(cxx, cxx_args)
        os.execv(path.join(prefix, "consumer"))

        -- The C consumer: the public header as ANSI C, linked against the shared C ABI package.
        -- The C ABI target only exists when the parser adapter is built.
        if with_fosu then
            local c_args = table.join({"-std=c89", p("tests/install/consumer.c"), "-o",
                                       path.join(prefix, "consumer_c")}, pkgflags("pppp_c"))
            os.execv(cc, c_args)

            -- The shared library is found through the loader path, not an RPATH in the consumer.
            if is_plat("macosx") then
                os.setenv("DYLD_LIBRARY_PATH", path.join(prefix, "lib"))
            else
                os.setenv("LD_LIBRARY_PATH", path.join(prefix, "lib"))
            end
            os.execv(path.join(prefix, "consumer_c"))
        end
    end)
task_end()
