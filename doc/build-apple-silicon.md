# Apple Silicon build notes

These notes describe the local development build recorded in October 2026. A clean-machine reproducible build and portable app packaging remain to be completed.

## Recorded dependencies

- Apple Silicon arm64 macOS, Xcode command line tools and Autotools.
- Qt 5.15.19, Boost 1.85, Protobuf 21, OpenSSL 4 and libevent from Homebrew.
- Berkeley DB 4.8.30.NC, built locally with its C++ interface and linked statically.
- The legacy Chia BLS/RELIC dependency expected by this Core version, built locally and installed to a separate prefix.
- libqrencode 4.1.1 for payment request QR codes. Check that configure enables `USE_QRCODE`.

The Berkeley DB source archive used locally had SHA-256:
`12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef`.

## Configuration outline

After preparing the dependencies, run `./autogen.sh` and configure with:

- `--with-gui=qt5 --enable-wallet --without-miniupnpc --disable-zmq`
- `--with-boost=<boost-prefix> --with-qt-bindir=<qt-prefix>/bin`
- `BDB_CFLAGS=-I<bdb-prefix>/include`
- `BDB_LIBS="-L<bdb-prefix>/lib -ldb_cxx-4.8"`
- `CPPFLAGS` / `LDFLAGS` pointing at the local BLS dependency include/library directories.
- `PKG_CONFIG_PATH` including the Protobuf, Qt, OpenSSL, libevent and libqrencode pkg-config directories.

Then build with `make -j<N>`. The graphical binary is `src/qt/machinecoin-qt`; launch with `-lang=en` for the English interface. Choose a separate data directory for development and testing.

These are configuration notes, not a complete dependency bootstrap script. In particular, the legacy BLS/RELIC and Berkeley DB dependency builds require their own platform preparation. Do not substitute unrelated newer cryptographic libraries without testing compatibility.

## Packaging still needed

Bundle the required dynamic libraries and Qt plugins, remove machine-specific paths, verify on a clean Apple Silicon Mac, and decide on signing/notarization before advertising a downloadable application. The source commit does not include the local `.app` bundle.
