{
  description = "A simple C++ OpenGL project using GLFW";

  inputs = {
      nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
      nixgl.url = "github:guibou/nixGL";
  };

  outputs = inputs:
    let
      system = "x86_64-linux";
      pkgs = import inputs.nixpkgs {
          inherit system;
          config.allowUnfree = true;
          overlays = [ inputs.nixgl.overlay ];
      };
      llvm = pkgs.llvmPackages_latest;
      nativeBuildInputs = with pkgs; [
        cmake
        gcc
        gnumake
        makeWrapper
      ];
      cmakePackages = with pkgs; [
        glfw
        glm
        libGL
        libGLU
        glbinding
        portaudio
        alsa-lib
        alsa-plugins
        libpulseaudio
        fftw
      ];
      buildTools = with pkgs; [
        nixgl.auto.nixGLDefault
        clang-tools
        llvm.clang
        llvm.libcxx
        boost
      ] ++ nativeBuildInputs;
      buildInputs = buildTools ++ cmakePackages;
      binaryDir = "$out/bin/visua";
      binaryPath = "$out/bin/visua";
    in
    {
      defaultPackage.${system} = pkgs.stdenv.mkDerivation {
        name = "visua";
        inherit buildInputs;
        inherit nativeBuildInputs;
        postInstall = pkgs.lib.optionalString pkgs.stdenv.isLinux ''
          # wrap binary so alsa can find its plugins
          mv $out/bin/visua $out/bin/_visua
          echo "#!/bin/sh" > $out/bin/visua
          echo "ALSA_PLUGIN_DIR=${pkgs.alsa-plugins}/lib/alsa-lib nixGL $out/bin/_visua" >> $out/bin/visua
          chmod a+x $out/bin/visua

          # copy over compile commands for clangd
          cp compile_commands.json $out/
        '';
        src = ./.;
      };

      devShell.${system} = pkgs.mkShell {
        buildInputs = buildInputs ++ [
          pkgs.pkg-config
        ];
      };
    };
}
