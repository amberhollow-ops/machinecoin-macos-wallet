# Machinecoin Wallet — Linux experimental preview 1

Community Midnight wallet for **Ubuntu 26.04, amd64 (Intel/AMD x86-64)**.
Not an official upstream release. Other Linux distributions, older Ubuntu
versions and ARM systems have not been validated.

## Install

Download the DEB and SHA256SUMS into the same directory. Verify the checksum:

```sh
sha256sum --ignore-missing -c SHA256SUMS
sudo apt install ./Machinecoin-Wallet-0.17.1-preview.1-ubuntu26.04-amd64.deb
```

Open **Machinecoin Wallet (Experimental)** from the application menu, or run
`machinecoin-wallet`. APT installs required Qt and other runtime libraries.
No daemon, background service, automatic mining or firewall changes are installed.
The first synchronization may take considerable time and disk space.

Back up your existing wallet before using this experimental version. Close any
other wallet/node using the same data directory before starting. The application
uses the existing Machinecoin default data directory; installation does not copy
or overwrite wallet data. Never publish wallet.dat, private keys or RPC credentials.

Uninstall with `sudo apt remove machinecoin-wallet-experimental`.
Wallet and blockchain data are not deleted by package removal.

## Source and build provenance

Based on fork commit `27d62728db32ff9fc8f9f780cf9a65203dca556d` plus
`linux-preview.patch`. The attached `Machinecoin-Linux-preview.1-source.tar.gz`
contains the tracked source with that patch already applied, and packaging helpers.
The release tag on the linux-preview branch includes these Linux source changes.
The explicitly attached Linux source archive also includes build-host helpers.

Built with GCC on Ubuntu 26.04, Qt 5, C++17, local static Boost 1.74 and Berkeley
DB 4.8, and local BLS dependencies. Qt, OpenSSL, protobuf and event libraries are
provided by Ubuntu packages. Third-party dependency source trees are not included
in the source archive. The included dependency scripts/configure command document
the build host and need their paths adapted for a fresh build; this is not a
turnkey reproducible-build environment.

## Validation and limitations

- The existing Linux wallet executable is packaged unchanged (no stripping or
  replacement). An incremental build check reported no work required.
- The owner reports successfully receiving coins with this Linux build. This is
  a user-confirmed mainnet receipt, not an independently repeated transaction test.
- DEB metadata/extraction and APT dependency resolution checked on the build host.
- Extracted packaged GUI started with a separate, offline regtest data directory
  and shut down cleanly. No live wallet was used for that check.
- Existing selected core run: 28 of 315 cases passed, 287 skipped; 91433 assertions
  passed. This was a previous build-host run, not a complete suite rerun.
- Existing Qt run still has failures in PaymentServerTests, RPCNestedTests and
  WalletTests. Do not interpret this preview as fully tested or production ready.
- Not tested on a clean Ubuntu installation. No new mainnet sending or mining
  validation was performed for this package.

Upstream: https://github.com/machinecoin-project/machinecoin-core
Community fork: https://github.com/amberhollow-ops/machinecoin-macos-wallet
