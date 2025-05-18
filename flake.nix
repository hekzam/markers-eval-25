{
  # Sources/dépendances externes
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";  # Paquets Nix standard de la branche instable
    flake-utils.url = "github:numtide/flake-utils";       # Utilitaires pour écrire des flakes
    flake-compat.url = "https://flakehub.com/f/edolstra/flake-compat/1.tar.gz";  # Pour la compatibilité avec Nix sans flake
  };

  # Définition des sorties de ce flake
  outputs = { self, nixpkgs, flake-utils, flake-compat }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        # Définition des paquets fournis par ce flake
        packages = rec {
          nixpkgs = pkgs;
          
          # Dérivation du paquet principal
          main = pkgs.stdenv.mkDerivation {
            pname = "hekzam-marker";
            version = if (self ? rev) then self.shortRev else self.dirtyShortRev;
            src = ./.;
            
            # Dépendances de construction
            nativeBuildInputs = with pkgs; [
              ninja
              pkg-config
              meson
            ];

            # Dépendances d'exécution
            buildInputs = with pkgs; [
              opencv
              zxing-cpp
              nlohmann_json
              (lib.getDev zbar)
            ];

            # Dépendances propagées dont les utilisateurs de ce paquet auront besoin
            propagatedBuildInputs = with pkgs; [
              python3
              typst
            ];

            # Étapes d'installation
            installPhase = ''
              mkdir -p $out/bin
              cp -r ./bench $out/bin/bench
              cp -r ./create-copy $out/bin/create-copy
              cp -r ./modifier $out/bin/modifier
              cp -r ./parser $out/bin/parser

              # cp -r ./* $out/bin
              if [ -f $out/bin/bench ]; then
                mv $out/bin/bench $out/bin/bench
              fi
            '';
          };

          # Paquet d'analyse pour le traitement statistique
          analysis = pkgs.stdenv.mkDerivation {
            pname = "hekzam-analysis";
            version = if (self ? rev) then self.shortRev else self.dirtyShortRev;

            src = ./stats-analysis;
            
            # Dépendances pour l'analyse statistique
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
              cp -r ./bench $out/bin/bench
              cp -r ./create-copy $out/bin/create-copy
              cp -r ./modifier $out/bin/modifier
              cp -r ./parser $out/bin/parser

              # cp -r ./* $out/bin
              if [ -f $out/bin/bench ]; then
                mv $out/bin/bench $out/bin/bench
              fi
            '';
          };

          # Définir le paquet par défaut comme étant le principal
          default = main;
        };

        # Environnements de développement
        devShells = {
          # L'environnement de développement par défaut étend le paquet principal avec des outils de développement
          default = packages.main.overrideAttrs (oldAttrs: {
            nativeBuildInputs = oldAttrs.nativeBuildInputs ++ [
              pkgs.gdb
              pkgs.llvmPackages_latest.clang-tools
            ];
          });
          
          # Environnement de test avec seulement le paquet principal
          test = pkgs.mkShell {
            packages = [
              packages.main
            ];   
          };
          
          # Environnement pour l'analyse statistique
          stats-analysis = pkgs.callPackage ./stats-analysis/shell.nix {
            inherit pkgs;
          };
        };
      }
    );
}


