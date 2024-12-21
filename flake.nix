{
  inputs = {
    nixpkgs.url = "nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};

      pico_sdk = pkgs.fetchgit {
        url = "https://github.com/raspberrypi/pico-sdk.git";
        rev = "1.5.1";
        hash = "sha256-GY5jjJzaENL3ftuU5KpEZAmEZgyFRtLwGVg3W1e/4Ho=";
        # rev = "master"; ## uncomment to use bleeding edge
        fetchSubmodules = true;
      };
    in {
      formatter = pkgs.alejandra;

      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          gcc-arm-embedded
          picocom # serial
          screen
		  python3 # required for cmake
		  libusb1 # bootsel
		  pkg-config
        ];

        shellHook = ''
          export PICO_SDK_PATH=${pico_sdk}
        '';
      };
    });
}
