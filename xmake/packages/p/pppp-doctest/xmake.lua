package("pppp-doctest")
    set_kind("library", { headeronly = true })
    set_description("doctest, pinned to the commit cmake/Doctest.cmake fetches")
    set_urls("https://github.com/doctest/doctest/archive/cd1d7474374c8cebed5c609564cdaa2f5184be25.tar.gz")
    add_versions("1.2.9", "cd9d23bba8247dfb47b132d7fb16414db72c451b1c51dd15d332dd064424250f")

    on_install(function (package)
        if package:is_plat("macosx") then
            os.vrun("patch -p1 -i " .. path.join(os.projectdir(), "tests/patches/doctest-arm64.patch"))
        end

        os.cp("doctest/doctest.h", package:installdir("include"))
        os.mkdir(package:installdir("include/doctest"))
        os.cp("doctest/parts", package:installdir("include/doctest"))
    end)
package_end()
