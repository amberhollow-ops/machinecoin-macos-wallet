#!/bin/bash
set -euo pipefail
SOURCE=$(cd "$(dirname "$0")/.." && pwd -P)
PREFIX=$(cd "${MACHINECOIN_DEPS_PREFIX:-$SOURCE/local-deps/prefix}" && pwd -P)
cd "$SOURCE"
MAP="-ffile-prefix-map=$SOURCE=/build/machinecoin -fdebug-prefix-map=$SOURCE=/build/machinecoin -fmacro-prefix-map=$SOURCE=/build/machinecoin -ffile-prefix-map=$PREFIX=/build/deps -fdebug-prefix-map=$PREFIX=/build/deps -fmacro-prefix-map=$PREFIX=/build/deps"
./autogen.sh
./configure --prefix=/usr --with-gui=qt5 --enable-tests --disable-bench --without-miniupnpc --disable-zmq --with-boost="$PREFIX" BDB_CFLAGS="-I$PREFIX/include" BDB_LIBS="-L$PREFIX/lib -ldb_cxx-4.8" CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib" CXXFLAGS="-O2 -g0 -std=c++17 $MAP" CFLAGS="-O2 -g0 -std=gnu11 $MAP"
make -j"${JOBS:-2}"
