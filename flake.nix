{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=23.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in rec {
        packages = rec {
          parser = pkgs.stdenv.mkDerivation {
            pname = "hekzam-parser";
            version = if (self ? rev) then self.shortRev else "dirty";
            src = pkgs.lib.sourceByRegex ./. [
              "^meson\.build"
              "^src"
              "^src/.*\.hpp"
              "^src/.*\.cpp"
            ];
            buildInputs = with pkgs; [
              opencv
              zxing-cpp
              nlohmann_json
            ];
            nativeBuildInputs = with pkgs; [
              meson
              ninja
              pkg-config
            ];
          };
          default = parser;
        };
        devShells = {};
      }
    );
}


