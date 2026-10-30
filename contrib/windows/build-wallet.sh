#!/usr/bin/env bash
set -euo pipefail
REPO=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
ROOT=$(dirname "$REPO")
PREFIX="${MACHINECOIN_DEPS_PREFIX:-$ROOT/deps/prefix}"
# Map both MSYS and native Windows paths used by GCC and its headers.
MAP=""
for base in "$REPO" "$PREFIX"; do
  native=$(cygpath -m "$base")
  for form in "$base" "$native"; do
    MAP="$MAP -ffile-prefix-map=$form=/build/machinecoin -fdebug-prefix-map=$form=/build/machinecoin -fmacro-prefix-map=$form=/build/machinecoin"
  done
done
export PATH="$PREFIX/bin:/mingw64/bin:/usr/bin:$PATH"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:/mingw64/lib/pkgconfig"
cd "$REPO"
./autogen.sh
./configure --build=x86_64-pc-msys --host=x86_64-w64-mingw32 \
  --with-gui=qt5 --enable-wallet --enable-tests --disable-bench --disable-zmq \
  --with-boost="$PREFIX" --with-boost-libdir="$PREFIX/lib" --with-qt-bindir=/mingw64/bin \
  --with-qt-incdir=/mingw64/include --with-qt-libdir=/mingw64/lib --without-qtdbus \
  --disable-maintainer-mode --disable-dependency-tracking --disable-shared \
  CC=gcc CXX=g++ AR=ar RANLIB=ranlib STRIP=strip WINDRES=windres \
  CXXFLAGS="-O2 -g0 -std=c++17 $MAP" CFLAGS="-O2 -g0 -std=gnu11 $MAP" \
  CPPFLAGS="-I$PREFIX/include -I$PREFIX/include/chiabls" \
  LDFLAGS="-L$PREFIX/lib -L/mingw64/lib" \
  BDB_CFLAGS="-I$PREFIX/include" BDB_LIBS="-L$PREFIX/lib -ldb_cxx-4.8" \
  LIBS='-lbcrypt -lws2_32 -lcrypt32 -lz'

make -j"${JOBS:-4}" LIBTOOL_APP_LDFLAGS=
