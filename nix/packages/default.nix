{ self, ... }:
{
  perSystem =
    { pkgs, config, ... }:
    {
      packages = {
        clangsharp-generator = pkgs.callPackage ./clangsharp-generator/package.nix { };

        pppp-c-win32 = pkgs.callPackage ./legacy-windows/package.nix {
          src = self;
          cross = pkgs.pkgsCross.mingw32;
          preset = "win2k";
          pname = "pppp-c-win32";
          windowsName = "2000 (x86)";
        };

        pppp-c-win64 = pkgs.callPackage ./legacy-windows/package.nix {
          src = self;
          cross = pkgs.pkgsCross.mingwW64;
          preset = "winxp-x64";
          pname = "pppp-c-win64";
          windowsName = "XP x64";
        };
      };
    };
}
