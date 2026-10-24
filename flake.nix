{
  description = "osu! difficulty and performance calculations in C++98, with Python, Node and C# bindings";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      # ClangSharp publishes no macOS x64 build, so x86_64-darwin cannot be evaluated here.
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ];

      imports = [
        ./nix/shells.nix
        ./nix/packages
      ];

      perSystem =
        { system, ... }:
        {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
            overlays = [ (import ./nix/overlays.nix) ];
          };
        };
    };
}
