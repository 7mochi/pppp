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
            pkgs.gcc
            pkgs.clang-tools
            pkgs.gnumake
            pkgs.git
            pkgs.pre-commit
            pkgs.nixfmt
            pkgs.dotnet-sdk
            (pkgs.python3.withPackages (ps: with ps; [ jinja2 ]))
          ]
          ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
            pkgs.bash
            pkgs.coreutils
          ];

          shellHook = ''
            echo "pp++"
            echo "  make configure                           # configure CMake"
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
            releasePkgs.cmake
            releasePkgs.ninja
            releasePkgs.gcc
            releasePkgs.gnumake
            releasePkgs.git
          ];

          shellHook = ''
            echo "pp++ / release"
            echo "  builds against glibc $(getconf GNU_LIBC_VERSION | cut -d' ' -f2)"
            echo "  cmake --preset release && cmake --build --preset release   # a Release build in build/release"
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
            echo "  cross-compiles pp++ from Linux with MinGW-w64"
            echo "  make build-windows-2k      # x86, Windows 2000 and later"
            echo "  make build-windows-xp-x64  # x64, Windows XP x64 and later"
          '';
        };
      };
    };
}
