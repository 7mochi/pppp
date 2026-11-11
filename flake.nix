{
  description = "osu! difficulty and performance calculations in C++98";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    nixpkgs-release.url = "github:nixos/nixpkgs/nixos-24.05";
  };

  outputs =
    inputs@{ flake-parts, nixpkgs-release, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ];

      imports = [
        ./nix/shells.nix
      ];

      perSystem =
        { system, ... }:
        {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
          };
          _module.args.releasePkgs = import nixpkgs-release {
            inherit system;
          };
        };
    };
}
