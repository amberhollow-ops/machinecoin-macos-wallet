#!/bin/bash
# Run from the dependency directory, with compatible bls-signatures sources present.
set -euo pipefail
DEPS=$(pwd -P)
PREFIX="${MACHINECOIN_DEPS_PREFIX:-$DEPS/prefix}"
MAP="-ffile-prefix-map=$DEPS=/build/deps -fdebug-prefix-map=$DEPS=/build/deps -fmacro-prefix-map=$DEPS=/build/deps"
cmake -S bls-signatures -B bls-signatures/build-private -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_C_FLAGS="-std=gnu11 $MAP" -DCMAKE_CXX_FLAGS="$MAP" -DCOMP="-O3 -funroll-loops -fomit-frame-pointer $MAP" -DCMAKE_INSTALL_PREFIX="$PREFIX"
cmake --build bls-signatures/build-private -j"${JOBS:-2}"
bls-signatures/build-private/src/runtest
mkdir -p "$PREFIX/lib"
# Use the combined archive from the build root, not src/libchiabls.a.
cp bls-signatures/build-private/libchiabls.a "$PREFIX/lib/"
# Existing dependency headers must match these sources.
