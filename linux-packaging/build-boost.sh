#!/bin/bash
# Run from the directory containing the dependency source trees.
set -eu
DEPS=$(pwd -P)
PREFIX="${MACHINECOIN_DEPS_PREFIX:-$DEPS/prefix}"
MAP="-ffile-prefix-map=$DEPS=/build/deps -fdebug-prefix-map=$DEPS=/build/deps -fmacro-prefix-map=$DEPS=/build/deps"
cd boost_1_74_0
./bootstrap.sh --with-libraries=system,filesystem,thread,chrono,program_options,test --prefix="$PREFIX"
./b2 -j2 variant=release link=static threading=multi cxxflags="-fPIC -std=c++11 $MAP" install
