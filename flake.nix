{
  description = "pp++: the library and its bindings";

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
                nodejs
                (python3.withPackages (ps: with ps; [
                  pytest
                  mypy
                  build
                  scikit-build-core
                  typing-extensions
                  pip
                ]))
              ])
              ++ (pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
                pkgs.bash
                pkgs.coreutils
              ]);

            shellHook = ''
              echo "pp++"
              echo "  make test"
              echo "  make format-check"
              echo "  npm install && npm test  # the node bindings"
            '';
          };
        }
      );
    };
}
