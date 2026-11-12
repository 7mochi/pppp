-- The three pkg-config packages users link against:
--   pppp    the calculation library
--   pppp_fosu  the calculation library and the parser
--   pppp_c  the C ABI shared library
--
-- A .pc file normally lists only its own library. This rule writes the two static packages with
-- every library they need, in the right order. The shared C ABI package does not list its
-- dependencies. The linker gets them from the shared library. Internal targets get no .pc file.
rule("pppp.pkgconfig")
    after_install(function (target)
        local pkgconfig_importfiles = import("target.action.install.pkgconfig_importfiles")
        local conf = target:extraconf("rules", "pppp.pkgconfig")
        local package = conf.package
        local links = {target:basename()}

        if conf.closure ~= false then
            local deps = target:orderdeps()

            for i = #deps, 1, -1 do
                local dep = deps[i]
                if dep:is_library() and not dep:is_headeronly() and not dep:is_shared() then
                    table.insert(links, dep:basename())
                end
            end
        end

        if is_plat("linux") then
            table.insert(links, "dl")
        end

        pkgconfig_importfiles(target, {
            filename = package .. ".pc",
            links = links,
        })

        local pcfile = path.join(target:libdir(), "pkgconfig", package .. ".pc")
        local text = io.readfile(pcfile)
        text = text:gsub("Name: [^\n]*", "Name: " .. package)
        text = text:gsub("Description: [^\n]*", "Description: " .. package)
        io.writefile(pcfile, text)
    end)
rule_end()

function pppp_install_pkgconfig(package, closure)
    add_rules("pppp.pkgconfig", {package = package, closure = closure ~= false})
end
