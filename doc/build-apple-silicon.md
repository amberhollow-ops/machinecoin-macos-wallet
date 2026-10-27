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

## Preview packaging

Preview 1 bundles Qt frameworks/plugins and Homebrew dynamic libraries using `macdeployqt`, with transitive Brotli and SharpYUV dependencies added explicitly. All Mach-O library references were checked for external Homebrew loads. Local debug symbols were stripped and BLS/RELIC rebuilt with `-ffile-prefix-map` (including RELIC's `COMP` flags) to remove personal diagnostic paths. Bundled licenses are under `Contents/Resources/Licenses`.

The [launcher](../contrib/macos-wallet-launcher.c) sets English and forwards user arguments without a hard-coded data directory. The app is ad-hoc signed and the DMG checked using `hdiutil verify`.

A bundled GUI smoke test created an isolated regtest wallet, mined three blocks, and shut down cleanly, with no external Homebrew libraries loaded. BLS tests passed (162 assertions, eight cases). These checks do not replace a clean-machine test or the unresolved full Core suite. Apple notarization is not provided.
