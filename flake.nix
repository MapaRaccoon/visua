{
  description = "A simple C++ OpenGL project using GLFW";

  inputs = {
      nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
      nixgl.url = "github:guibou/nixGL";
      nixpkgs-with-nvidia-driver-fix.url = "github:nixos/nixpkgs/pull/222762/head";
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
      buildInputs = with pkgs; [
        gcc
        cmake
        gnumake
        glfw
        glm
        libGL
        libGLU
        glbinding
        nixgl.auto.nixGLDefault
        clang-tools
        llvm.clang
        llvm.libcxx
      ];
    in
    {
      defaultPackage.${system} = pkgs.stdenv.mkDerivation {
        name = "visua";
        inherit buildInputs;
        src = ./.;
        installPhase = ''
          mkdir -p $out/bin
          cp main $out/bin/main
        '';
      };

      devShell.${system} = pkgs.mkShell {
        buildInputs = buildInputs ++ [
          pkgs.pkg-config
        ];
      };
    };
}
