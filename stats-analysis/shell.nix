{pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  pname = "stats-analysis-shell";
  packages = with pkgs;
    [     
      python3
      R
      rPackages.ggplot2
      rPackages.dplyr
      rPackages.tidyr
      rPackages.fs
      rPackages.RColorBrewer
      rPackages.gridExtra
  ];
}