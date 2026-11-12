-- Each target gets its own flags. The library turns exceptions off, but doctest needs them.

-- fosu has some warnings we leave alone. Add these flags only to targets that include fosu, and
-- only for the compiler that needs them.
local function pppp_compiler_suppressions(parser)
    on_config(function (target)
        local compiler = target:compiler("cxx")
        local name = compiler and compiler:name() or ""
        if name:match("clang") then
            if parser then
                -- Clang warns about unnamed template arguments and about memcpy on a C++ class.
                target:add("cxflags", "-Wno-unnamed-type-template-args", "-Wno-nontrivial-memcall")
            end
        elseif parser and (name:match("gcc") or name:match("gxx") or name:match("mingw")) then
            -- GCC warns about a value that may be uninitialized and about memcpy on a C++ class.
            target:add("cxflags", "-Wno-maybe-uninitialized", "-Wno-class-memaccess")
        end
    end)
end

function pppp_library_flags(parser)
    set_warnings("allextra", "pedantic")

    if pppp.warnings_as_errors then
        set_warnings("error")
    end

    pppp_compiler_suppressions(parser)

    if is_plat("windows") then
        -- Match the other compilers. Use precise floating point, and turn off exceptions and RTTI.
        add_cxflags("/fp:precise", "/GR-", "/EHs-c-")

        if is_arch("x86") then
            -- 32-bit x86 without SSE2 uses x87 and gives different numbers.
            add_cxflags("/arch:SSE2")
        end
    else
        -- The 64-bit code uses long long, which C++98 rejects.
        add_cxflags("-Wshadow", "-Wundef", "-Wcast-align", "-Wno-long-long")
        -- Turn off fused multiply-add because it changes the results.
        add_cxflags("-ffp-contract=off")

        if is_arch("x86") then
            -- 32-bit x86 without SSE2 uses x87 and gives different numbers.
            add_cxflags("-msse2", "-mfpmath=sse")
        end

        -- Turn off exceptions and RTTI. Sections for each function and data item let the linker
        -- drop unused code.
        add_cxflags("-fno-exceptions", "-fno-rtti", "-ffunction-sections", "-fdata-sections")

        if is_plat("macosx") then
            add_ldflags("-Wl,-dead_strip")
        else
            add_ldflags("-Wl,--gc-sections")
        end
    end
end

function pppp_test_flags(parser)
    set_warnings("allextra", "pedantic")

    if pppp.warnings_as_errors then
        set_warnings("error")
    end

    pppp_compiler_suppressions(parser)

    if not is_plat("windows") then
        -- The library's warning and floating-point flags, but exceptions stay on.
        add_cxflags("-Wshadow", "-Wundef", "-Wcast-align", "-Wno-long-long")
        add_cxflags("-ffp-contract=off")

        if is_arch("x86") then
            add_cxflags("-msse2", "-mfpmath=sse")
        end
    end
end

function pppp_parser_test_flags()
    set_warnings("allextra", "pedantic")

    if pppp.warnings_as_errors then
        set_warnings("error")
    end

    pppp_compiler_suppressions(true)

    if not is_plat("windows") then
        add_cxflags("-ffp-contract=off")

        if is_arch("x86") then
            add_cxflags("-msse2", "-mfpmath=sse")
        end
    end
end

function pppp_doctest_flags()
    if pppp.doctest_dir ~= "" then
        add_sysincludedirs(pppp.doctest_dir, { public = true })
    else
        add_packages("pppp-doctest")
    end

    add_defines("DOCTEST_CONFIG_NO_POSIX_SIGNALS")
end

function pppp_cjson_flags()
    if pppp.cjson_dir ~= "" then
        add_sysincludedirs(pppp.cjson_dir, { public = true })
    else
        add_packages("pppp-cjson")
    end
end
