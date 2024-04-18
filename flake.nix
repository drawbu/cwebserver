{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = {self, ...} @ inputs:
    inputs.utils.lib.eachDefaultSystem (system: let
      pkgs = inputs.nixpkgs.legacyPackages.${system};
      cc = pkgs.gcc12;
    in rec {
      formatter = pkgs.alejandra;

      devShells.default = pkgs.mkShell {
        packages = with pkgs; [
          criterion
          gcovr
          valgrind
          bear
          gdb
        ];

        inputsFrom = [packages.default];
      };

      packages = {
        default = packages.server;
        server = pkgs.stdenv.mkDerivation {
          name = "server";
          src = ./.;
          buildInputs = [cc] ++ (with pkgs; [glibc gnumake]);
          installPhase = ''
            mkdir -p $out/bin
            install -D server $out/bin/server
          '';
        };
      };
    });
}
