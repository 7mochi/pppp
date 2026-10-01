{
  description = "pp++: the library and its Python bindings";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";

  outputs =
    { nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-darwin" ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
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
                python3
                python3Packages.pytest
                python3Packages.mypy
                python3Packages.build
                python3Packages.scikit-build-core
                python3Packages.typing-extensions
              ])
              ++ (pkgs.lib.optionals pkgs.stdenv.isDarwin [
                pkgs.bash
                pkgs.coreutils
              ]);

            shellHook = ''
              echo "pp++"
              echo "  make test"
              echo "  make check"
            '';
          };
        }
      );
    };
}
