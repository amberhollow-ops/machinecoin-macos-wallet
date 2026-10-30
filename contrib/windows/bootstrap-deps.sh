#!/usr/bin/env bash
# Run from MSYS2 with the native MINGW64 packages installed.
set -euo pipefail
REPO=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
WORK=${WORK:-"$(dirname "$REPO")/deps"}
PREFIX="$WORK/prefix"
export PATH=/mingw64/bin:/usr/bin:$PATH
mkdir -p "$WORK" "$PREFIX"
cd "$WORK"
MAP=""
for form in "$(pwd -P)" "$(cygpath -m "$WORK")"; do
  MAP="$MAP -ffile-prefix-map=$form=/build/deps -fdebug-prefix-map=$form=/build/deps -fmacro-prefix-map=$form=/build/deps"
done
fetch() {
  local name=$1 url=$2 hash=$3
  test -f "$name" || curl -fL --retry 3 -o "$name" "$url"
  echo "$hash  $name" | sha256sum -c -
}
fetch bdb.tar.gz https://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz 12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef
fetch bls.tar.gz https://github.com/codablock/bls-signatures/archive/v20181101.tar.gz b3ec74a77a7b6795f84b05e051a0824ef8d9e05b04b2993f01040f35689aa87c
fetch boost.tar.gz https://archives.boost.io/release/1.74.0/source/boost_1_74_0.tar.gz afff36d392885120bcac079148c177d1f6f7730ec3d47233aa51b0afa4db94a5
fetch protobuf.tar.gz https://github.com/protocolbuffers/protobuf/releases/download/v21.12/protobuf-all-21.12.tar.gz 2c6a36c7b5a55accae063667ef3c55f2642e67476d96d355ff0acb13dbb47f09
test -d db-4.8.30.NC || tar xf bdb.tar.gz
test -d bls-signatures-20181101 || tar xf bls.tar.gz
test -d boost_1_74_0 || tar xf boost.tar.gz
test -d protobuf-21.12 || tar xf protobuf.tar.gz

cd "$WORK/db-4.8.30.NC"
# The upstream tarball has read-only files; make only this extracted tree writable.
python -c 'import os,stat; [(os.chmod(os.path.join(r,f),stat.S_IREAD|stat.S_IWRITE)) for r,ds,fs in os.walk(".") for f in fs]'
sed -i 's/__atomic_compare_exchange(/__atomic_compare_exchange_db(/g' dbinc/atomic.h
sed -i 's/atomic_init(/atomic_init_db(/g' dbinc/atomic.h mp/mp_region.c mp/mp_mvcc.c mp/mp_fget.c mutex/mut_method.c mutex/mut_tas.c
cp "$REPO/depends/config.guess" "$REPO/depends/config.sub" dist/
mkdir -p build_windows
cd build_windows
../dist/configure --build=x86_64-pc-msys --host=x86_64-w64-mingw32 --prefix="$PREFIX" --enable-mingw --disable-shared --enable-cxx --disable-replication CC=gcc CXX=g++ AR=ar RANLIB=ranlib CFLAGS="-O2 -std=gnu11 $MAP" CXXFLAGS="-O2 -std=c++11 $MAP"
make -j8 libdb_cxx-4.8.a libdb-4.8.a
make install_lib install_include

cd "$WORK/bls-signatures-20181101"
# The portable reference BLAKE2 code does not need over-aligned packed states.
# Modern GCC rejects arrays of these legacy packed/over-aligned typedefs.
sed -i 's/ALIGNME( 64 ) typedef struct/typedef struct/' contrib/relic/src/md/blake2.h
cmake -S . -B build-native -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_INSTALL_PREFIX="$(cygpath -m "$PREFIX")" -DOPSYS=WINDOWS -DWSIZE=64 -DSTLIB=ON -DSHLIB=OFF -DSTBIN=ON -DCMAKE_C_FLAGS="-std=gnu11 $MAP" -DCMAKE_CXX_FLAGS="$MAP" -DCOMP="-O3 -funroll-loops -fomit-frame-pointer $MAP" -DCMAKE_BUILD_TYPE=Release
cmake --build build-native --target chiablstmp relic_s runtest -j8
./build-native/src/runtest.exe
mkdir -p build-native/merged "$PREFIX/lib" "$PREFIX/include/chiabls"
cd build-native/merged
ar x ../src/libchiablstmp.a
ar x ../contrib/relic/lib/librelic_s.a
ar crs "$PREFIX/lib/libchiabls.a" *.obj
cd ../..
cp src/*.hpp contrib/relic/include/*.h build-native/contrib/relic/include/relic_conf.h "$PREFIX/include/chiabls/"

cd "$WORK/boost_1_74_0"
./bootstrap.sh --with-toolset=gcc
./b2 -j8 toolset=gcc target-os=windows address-model=64 variant=release link=static threading=multi threadapi=win32 cxxflags="$MAP" --layout=system --with-system --with-filesystem --with-thread --with-chrono --with-program_options --with-test --with-date_time --with-atomic --prefix="$(cygpath -m "$PREFIX")" install

cd "$WORK/protobuf-21.12"
cmake -S cmake -B build-native -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$(cygpath -m "$PREFIX")" -DCMAKE_C_FLAGS="$MAP" -DCMAKE_CXX_FLAGS="$MAP" -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF
cmake --build build-native -j8
cmake --install build-native
