{ pkgs, cross }:

# the default posix thread model needs libgcc symbols that windows 2000 and xp reject
pkgs.overrideCC cross.stdenv (
  cross.stdenv.cc.override (old: {
    cc = old.cc.override {
      threadsCross = {
        model = "win32";
        package = null;
      };
    };
  })
)
