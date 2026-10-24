{
  lib,
  src,
  pkgs,
  cross,
  preset,
  pname,
  windowsName,
}:

(import ../../lib/win32-threads.nix { inherit pkgs cross; }).mkDerivation {
  inherit pname;
  version = "0.0.0";
  inherit src;

  nativeBuildInputs = [
    pkgs.cmake
    pkgs.ninja
  ];

  dontUseCmakeConfigure = true;

  # patchelf and the ELF strip would corrupt a PE file
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

  meta = with lib; {
    description = "C ABI shared library for Windows ${windowsName}";
    homepage = "https://github.com/7mochi/pppp";
    license = licenses.mit;
  };
}
