{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    flake-compat.url = "https://flakehub.com/f/edolstra/flake-compat/1.tar.gz";

  };

  outputs = { self, nixpkgs, flake-utils,flake-compat }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = rec {
          nixpkgs = pkgs;
          parser = pkgs.stdenv.mkDerivation {
            pname = "hekzam-parser";
            version = if (self ? rev) then self.shortRev else self.dirtyShortRev;

            # src = pkgs.lib.sourceByRegex ./. [
            #   "^(src|include)"
            #   "^(src|include)/.*\.(h|hpp)"
            #   "^(src|include)/.*\.(c|cpp)"
            #   "^(typst).*"
            #   "^.*\.sh"
            #   "^CMakeLists\.txt"
            # ];
            src = ./.;
            
            nativeBuildInputs = with pkgs; [
              # cmake
              ninja
              pkg-config
              meson
            ];

            buildInputs = with pkgs; [
              opencv
              zxing-cpp
              nlohmann_json
              (lib.getDev zbar)
            ];

            propagatedBuildInputs = with pkgs; [
              typst
            ];

            # cmakeFlags = [
            #   "-G Ninja"
            #   "-DCMAKE_BUILD_TYPE=Release"
            #   "-DENABLE_ZBAR=ON"
            # ];

            installPhase = ''
              mkdir -p $out/bin
              cp -r ./benchmarker $out/bin/benchmark
              cp -r ./typst_interface $out/bin/typst_interface
              cp -r ./modifier $out/bin/modifier
              cp -r ./parser $out/bin/parser

              # cp -r ./* $out/bin
              if [ -f $out/bin/benchmarker ]; then
                mv $out/bin/benchmarker $out/bin/benchmark
              fi
            '';
          };

          analysis = pkgs.stdenv.mkDerivation {
            pname = "hekzam-analysis";
            version = if (self ? rev) then self.shortRev else self.dirtyShortRev;

            src = ./stats-analysis;
            
            buildInputs = with pkgs; [
              python3
              R
              rPackages.ggplot2
              rPackages.dplyr
              rPackages.tidyr
              rPackages.fs
              rPackages.RColorBrewer
              rPackages.gridExtra
            ];

            installPhase = ''
              mkdir -p $out/bin
              cp -r ./benchmarker $out/bin/benchmark
              cp -r ./typst_interface $out/bin/typst_interface
              cp -r ./modifier $out/bin/modifier
              cp -r ./parser $out/bin/parser

              # cp -r ./* $out/bin
              if [ -f $out/bin/benchmarker ]; then
                mv $out/bin/benchmarker $out/bin/benchmark
              fi
            '';
          };

          default = parser;
        };

        devShells = {
          default = packages.parser.overrideAttrs (oldAttrs: {
            nativeBuildInputs = oldAttrs.nativeBuildInputs ++ [
              pkgs.gdb
              pkgs.llvmPackages_latest.clang-tools
            ];
          });
          test = pkgs.mkShell {
            packages = [
              packages.parser
             ];   
          };
          stats-analysis = pkgs.callPackage ./stats-analysis/shell.nix {
            inherit pkgs;
          };
        };
      }
    );
}


