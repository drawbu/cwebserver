{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = {self, ...} @ inputs:
    inputs.utils.lib.eachDefaultSystem (system: let
      pkgs = inputs.nixpkgs.legacyPackages.${system};
      cc = pkgs.gcc12;
    in {
      formatter = pkgs.alejandra;

      devShells.default = pkgs.mkShell {
        packages =
          [cc]
          ++ (with pkgs; [
            glibc
            gnumake
            criterion
            gcovr
            valgrind
            bear
            gdb
          ]);
      };
    });
}
