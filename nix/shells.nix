{
  perSystem =
    { pkgs, config, ... }:
    {
      devShells = {
        default = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.ninja
            pkgs.gcc
            pkgs.clang-tools
            pkgs.gnumake
            pkgs.nodejs
            pkgs.git
            pkgs.pre-commit
            pkgs.nixfmt
            pkgs.dotnet-sdk
            pkgs.biome
            config.packages.clangsharp-generator
            pkgs.libclang.lib
            (pkgs.python3.withPackages (
              ps: with ps; [
                pytest
                mypy
                build
                scikit-build-core
                typing-extensions
                pip
                jinja2
                clang
                ctypesgen
              ]
            ))
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
            echo "  pip install . && pytest python/tests     # python bindings"
            echo "  npm install && npm test                  # node bindings"
            echo "  cd csharp/tests && dotnet run -f net8.0  # c# bindings"
            ${pkgs.lib.optionalString pkgs.stdenv.hostPlatform.isLinux ''
              echo "  nix develop .#legacyWindows              # legacy windows targets"
            ''}if [ -t 1 ] && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
              pre-commit install >/dev/null 2>&1 || true
            fi
          '';
        };
      }
      // pkgs.lib.optionalAttrs pkgs.stdenv.hostPlatform.isLinux {
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
