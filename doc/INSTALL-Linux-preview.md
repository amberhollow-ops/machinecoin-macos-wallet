# Machinecoin Wallet — Linux experimental preview 2

Community Midnight wallet for **Ubuntu 26.04, amd64 (Intel/AMD x86-64)**.
Not an official upstream release. Other Linux distributions, older Ubuntu
versions and ARM systems have not been validated.

## Install

Download the DEB and SHA256SUMS into the same directory. Verify the checksum:

```sh
sha256sum --ignore-missing -c SHA256SUMS
sudo apt install ./Machinecoin-Wallet-0.17.1-preview.2-ubuntu26.04-amd64.deb
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
`linux-preview.patch`. The attached `Machinecoin-Linux-preview.2-source.tar.gz`
contains the tracked source with that patch already applied, and packaging helpers.
The source archive includes portable packaging helpers. This is a release candidate; publication and tag updates are separate steps.

Built with GCC on Ubuntu 26.04, Qt 5, C++17, local static Boost 1.74 and Berkeley
DB 4.8, and local BLS dependencies. Qt, OpenSSL, protobuf and event libraries are
provided by Ubuntu packages. Third-party dependency source trees are not included
in the source archive. The included dependency scripts resolve paths at runtime. Dependency sources and
compatibility patches must be prepared separately; this is not a turnkey
reproducible-build environment.

## Validation and limitations

- This candidate recompiles the application and its BLS dependency with local path remapping. No consensus or payment rules are changed.
- See the accompanying VALIDATION.txt for tests performed on this exact candidate.
- Earlier previews had known failures in legacy wallet/payment test fixtures. A complete passing test suite is not claimed.
- Not tested on a clean Ubuntu installation. No mainnet sending or mining validation is claimed.

Upstream: https://github.com/machinecoin-project/machinecoin-core
Community fork: https://github.com/amberhollow-ops/machinecoin-macos-wallet

Privacy rebuild: application compilation remaps local build paths. Release candidates must pass a complete extracted-package path scan before publication. Earlier test results describe the previous build; see the accompanying new validation report.
