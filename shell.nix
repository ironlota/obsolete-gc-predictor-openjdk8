with import <nixpkgs> { };

lib.overrideDerivation (import ./.) (attrs: {
  src = null;
  buildInputs = attrs.buildInputs ++ [ ];
})
