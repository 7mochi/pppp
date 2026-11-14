package("pppp-cjson")
    set_kind("library")
    set_description("cJSON, pinned to a fixed commit")
    set_urls("https://github.com/DaveGamble/cJSON/archive/c859b25da02955fef659d658b8f324b5cde87be3.tar.gz")
    add_versions("1.7.18", "880a86445ad5cf38f877bac31198268d8fac5d2ce996ebf55d3066a4109ddd8d")

    on_install(function (package)
        import("package.tools.cmake").install(package,
            { "-DENABLE_CJSON_TEST=OFF", "-DBUILD_SHARED_LIBS=OFF" })

        os.cp(path.join(package:installdir("include"), "cjson", "cJSON.h"),
              package:installdir("include"))
    end)
package_end()
