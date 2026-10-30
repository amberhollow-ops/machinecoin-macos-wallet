#!/bin/bash
# Run from the directory containing the dependency source trees.
set -eu
DEPS=$(pwd -P)
PREFIX="${MACHINECOIN_DEPS_PREFIX:-$DEPS/prefix}"
MAP="-ffile-prefix-map=$DEPS=/build/deps -fdebug-prefix-map=$DEPS=/build/deps -fmacro-prefix-map=$DEPS=/build/deps"
cd db-4.8.30.NC/build_unix
../dist/configure --enable-cxx --disable-shared --disable-replication --with-pic --prefix="$PREFIX" CFLAGS="-O2 -std=gnu11 -Wno-error=implicit-int -Wno-error=implicit-function-declaration $MAP" CXXFLAGS="-O2 -std=c++11 $MAP"
make -j2 libdb_cxx-4.8.a libdb-4.8.a
make install_lib install_include
