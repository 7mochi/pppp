-- The two pkg-config packages users link against:
--   pkg-config --cflags --libs pppp   -- the calculation library
--   pkg-config --cflags --libs fosu   -- the calculation library + the parser
-- A *.pc file normally lists only its own target's library. This rule writes both public files with
-- every library they need, in the right order. The internal targets get no *.pc file.
rule("pppp.pkgconfig")
    after_install(function (target)
        local pkgconfig_importfiles = import("target.action.install.pkgconfig_importfiles")
        local package = target:extraconf("rules", "pppp.pkgconfig").package
        local links = {target:basename()}
        local deps = target:orderdeps()

        for i = #deps, 1, -1 do
            local dep = deps[i]
            if dep:is_library() and not dep:is_headeronly() and not dep:is_shared() then
                table.insert(links, dep:basename())
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

function pppp_install_pkgconfig(package)
    add_rules("pppp.pkgconfig", {package = package})
end
