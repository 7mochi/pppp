{
  lib,
  stdenv,
  fetchurl,
  unzip,
  autoPatchelfHook,
  makeWrapper,
  zlib,
  zstd,
}:

let
  version = "22.1.8";

  # nuget calls platforms RIDs
  runtimeId =
    {
      x86_64-linux = "linux-x64";
      aarch64-linux = "linux-arm64";
      aarch64-darwin = "osx-arm64";
    }
    .${stdenv.hostPlatform.system};

  libraryPathVariable =
    if stdenv.hostPlatform.isDarwin then "DYLD_LIBRARY_PATH" else "LD_LIBRARY_PATH";

in
stdenv.mkDerivation {
  pname = "clangsharp-generator";
  inherit version;

  src = fetchurl {
    url = "https://api.nuget.org/v3-flatcontainer/clangsharppinvokegenerator.${runtimeId}/${version}/clangsharppinvokegenerator.${runtimeId}.${version}.nupkg";
    hash =
      {
        linux-x64 = "sha256-wI+Oru41+1g+IkVffj9vZF4wXqrSUoB5aU6wLwyknfE=";
        linux-arm64 = "sha256-KWZK6p0jAoG2/Uks1DAd2pmaE4skvVtFBiyun/iNB/w=";
        osx-arm64 = "sha256-xjpOnscdSSKpyqp51glQXGWiWVLh2QLWOvrJ2DbECfg=";
      }
      .${runtimeId};
  };

  # autoPatchelfHook is ELF-only.
  nativeBuildInputs = [
    unzip
    makeWrapper
  ]
  ++ lib.optionals stdenv.hostPlatform.isLinux [ autoPatchelfHook ]
  ++ lib.optionals stdenv.hostPlatform.isDarwin [ stdenv.cc.bintools ];

  # what the bundled libClangSharp links against
  buildInputs = [
    zlib
    zstd
    stdenv.cc.cc.lib
  ];

  dontBuild = true;
  dontConfigure = true;

  # the package expects homebrew and a versioned dylib name
  postFixup = lib.optionalString stdenv.hostPlatform.isDarwin ''
    ln -s libClangSharp.dylib $out/lib/libClangSharp.${version}.dylib
    install_name_tool -change /opt/homebrew/opt/zstd/lib/libzstd.1.dylib \
      ${zstd.out}/lib/libzstd.1.dylib $out/lib/libClangSharp.dylib
    install_name_tool -add_rpath @loader_path $out/lib/libClangSharp.dylib
  '';

  unpackPhase = ''
    runHook preUnpack
    unzip -q "$src" -d source
    runHook postUnpack
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin $out/lib
    cp -r source/tools/any/${runtimeId}/. $out/lib/

    ${lib.optionalString stdenv.hostPlatform.isLinux ''
      # the binary wants libclang.so.22.1; the package ships only libclang.so
      ln -s libclang.so $out/lib/libclang.so.22.1
    ''}

    chmod +x $out/lib/ClangSharpPInvokeGenerator
    makeWrapper $out/lib/ClangSharpPInvokeGenerator $out/bin/ClangSharpPInvokeGenerator \
      --prefix ${libraryPathVariable} : "$out/lib" \
      --set DOTNET_SYSTEM_GLOBALIZATION_INVARIANT 1

    runHook postInstall
  '';

  meta = with lib; {
    description = "Generates C# P/Invoke bindings from C headers";
    homepage = "https://github.com/dotnet/ClangSharp";
    license = licenses.mit;
    platforms = [
      "x86_64-linux"
      "aarch64-linux"
      "aarch64-darwin"
    ];
    mainProgram = "ClangSharpPInvokeGenerator";
    sourceProvenance = [ sourceTypes.binaryNativeCode ];
  };
}
