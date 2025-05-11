let 

flake = (import
  (
    let
      lock = builtins.fromJSON (builtins.readFile ./flake.lock);
      nodeName = lock.nodes.root.inputs.flake-compat;
    in
    fetchTarball {
      url = lock.nodes.${nodeName}.locked.url or "https://github.com/edolstra/flake-compat/archive/${lock.nodes.${nodeName}.locked.rev}.tar.gz";
      sha256 = lock.nodes.${nodeName}.locked.narHash;
    }
  )
  { src = ./.; }
);

pkgs = flake.outputs.packages.x86_64-linux.nixpkgs;
    
in
pkgs.mkShell  {
  packages = [
      pkgs.python3
      pkgs.R
      pkgs.rPackages.ggplot2
      pkgs.rPackages.dplyr
      pkgs.rPackages.tidyr
      pkgs.rPackages.fs
      pkgs.rPackages.RColorBrewer
      pkgs.rPackages.gridExtra
      flake.outputs.packages.x86_64-linux.default
  ];
}
