{
  description = "pp++: the library and its bindings";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";

  outputs =
    { nixpkgs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-darwin"
      ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
      pkgs = nixpkgs.legacyPackages.x86_64-linux;

      useWin32ThreadModel =
        cross:
        pkgs.overrideCC cross.stdenv (
          cross.stdenv.cc.override (old: {
            cc = old.cc.override {
              threadsCross = {
                model = "win32";
                package = null;
              };
            };
          })
        );

      legacyWindowsFor =
        {
          cross,
          preset,
          pname,
        }:
        (useWin32ThreadModel cross).mkDerivation {
          inherit pname;
          version = "0.0.0";

          src = ./.;

          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
          ];

          dontUseCmakeConfigure = true;

          # A PE file must not go through patchelf or the ELF strip after the build.
          dontFixup = true;
          dontStrip = true;

          configurePhase = ''
            runHook preConfigure
            cmake --preset ${preset}
            runHook postConfigure
          '';

          buildPhase = ''
            runHook preBuild
            cmake --build --preset ${preset}
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out
            cp build/${preset}/pppp_c.dll $out/
            runHook postInstall
          '';
        };
    in
    {
      packages.x86_64-linux = {
        pppp-c-win32 = legacyWindowsFor {
          cross = pkgs.pkgsCross.mingw32;
          preset = "win2k";
          pname = "pppp-c-win32";
        };

        pppp-c-win64 = legacyWindowsFor {
          cross = pkgs.pkgsCross.mingwW64;
          preset = "winxp-x64";
          pname = "pppp-c-win64";
        };
      };

      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.mkShell {
            packages =
              (with pkgs; [
                cmake
                ninja
                gcc
                clang-tools
                gnumake
                nodejs
                dotnet-sdk_8
                (python3.withPackages (
                  ps: with ps; [
                    pytest
                    mypy
                    build
                    scikit-build-core
                    typing-extensions
                    pip
                  ]
                ))
              ])
              ++ (pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
                pkgs.bash
                pkgs.coreutils
              ]);

            shellHook = ''
              echo "pp++"
              echo "  make configure                           # configure CMake"
              echo "  make test                                # build and run the suites"
              echo "  make format                              # C++ formatting"
              echo "  make check-format                        # C++ formatting, checked"
              echo "  make check-tidy                          # C++ analysis, report only"
              echo "  pip install . && pytest python/tests     # python bindings"
              echo "  npm install && npm test                  # node bindings"
              echo "  cd csharp/tests && dotnet run -f net8.0  # c# bindings"
              echo "  nix develop .#legacyWindows              # legacy windows targets"
            '';
          };
        }
        // nixpkgs.lib.optionalAttrs (system == "x86_64-linux") {
          legacyWindows = pkgs.mkShell {
            packages = [
              pkgs.cmake
              pkgs.ninja
              pkgs.gnumake
              (useWin32ThreadModel pkgs.pkgsCross.mingw32).cc
              (useWin32ThreadModel pkgs.pkgsCross.mingwW64).cc
            ];

            shellHook = ''
              echo "pp++ / legacy Windows"
              echo "  cross-compiles pp++ from Linux with MinGW-w64"
              echo "  make build-windows-2k      # x86, Windows 2000 and later"
              echo "  make build-windows-xp-x64  # x64, Windows XP x64 and later"
            '';
          };
        }
      );
    };
}
