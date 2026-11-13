{
  perSystem =
    {
      pkgs,
      releasePkgs,
      ...
    }:
    {
      devShells = {
        default = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.ninja
            pkgs.xmake
            pkgs.gcc
            pkgs.clang-tools
            pkgs.gnumake
            pkgs.pkg-config
            pkgs.git
            pkgs.pre-commit
            pkgs.nixfmt
            pkgs.dotnet-sdk
            (pkgs.callPackage ./packages/clangsharp-generator/package.nix { })
            pkgs.libclang.lib
            (pkgs.python3.withPackages (ps: with ps; [ jinja2 ]))
          ]
          ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
            pkgs.bash
            pkgs.coreutils
          ];

          shellHook = ''
            echo "pp++"
            echo "  make configure                           # configure xmake"
            echo "  make test-auto                           # build and run the suites"
            echo "  make format                              # C++ formatting"
            echo "  make check-format                        # C++ formatting, checked"
            echo "  make check-tidy                          # C++ analysis, report only"
            ${pkgs.lib.optionalString pkgs.stdenv.hostPlatform.isLinux ''
              echo "  nix develop .#legacyWindows              # legacy windows targets"
            ''}if [ -t 1 ] && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
              pre-commit install >/dev/null 2>&1 || true
            fi
          '';
        };
      }
      // pkgs.lib.optionalAttrs pkgs.stdenv.hostPlatform.isLinux {
        # nixos-24.05's libc: loads on any newer glibc
        release = releasePkgs.mkShell {
          packages = [
            pkgs.xmake
            releasePkgs.gcc
            releasePkgs.gnumake
            releasePkgs.git
          ];

          shellHook = ''
            echo "pp++ / release"
            echo "  builds against glibc $(getconf GNU_LIBC_VERSION | cut -d' ' -f2)"
            echo "  xmake f -m release && xmake   # a Release build in build/"
          '';
        };

        legacyWindows = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.ninja
            pkgs.gnumake
            (import ./lib/win32-threads.nix {
              inherit pkgs;
              cross = pkgs.pkgsCross.mingw32;
            }).cc
            (import ./lib/win32-threads.nix {
              inherit pkgs;
              cross = pkgs.pkgsCross.mingwW64;
            }).cc
          ];

          shellHook = ''
            echo "pp++ / legacy Windows"
            echo "  cross-compilation is deferred to the xmake port"
            echo "  the MinGW-w64 compilers are provisioned for that follow-up"
          '';
        };
      };
    };
}
