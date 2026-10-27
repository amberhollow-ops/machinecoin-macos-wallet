# Machinecoin Wallet for Apple Silicon

Experimental native macOS arm64 adaptation of [Machinecoin Core](https://github.com/machinecoin-project/machinecoin-core), based on upstream commit `fddd907052cb03c1907f0a57037c37c5388c9452` (v0.17.1 Octopus).

## Included changes

- Apple Silicon Objective-C message dispatch fixes and compatibility updates for current Boost, OpenSSL and C++ toolchains.
- Midnight dark interface with amber accents, sidebar navigation, balance cards, peer status and local chain status.
- Payment request QR support when built with libqrencode, plus case-preserving `machinecoin://` URI normalization.
- Opt-in CPU solo mining with thread selection, explicit start/stop controls and synchronization readiness checks. No Apple Metal GPU miner is included.
- Preserve healthy peers on sparse networks instead of disconnecting them solely because of a cached masternode full-sync marker.

## Build and release status

This repository publishes source code. It is **not a notarized, standalone macOS app release**. The locally tested application uses Homebrew libraries and an ad-hoc signature; copying that app to another Mac is not a supported installation method yet.

See [Apple Silicon build notes](doc/build-apple-silicon.md) for the recorded environment and remaining packaging work. The older upstream build guides are retained for reference and are not a verified Apple Silicon recipe.

## Validation status

The local arm64 application was built and exercised in October 2026. Recorded checks include wallet database, wallet cryptography, accounting, coin selection, BLS/crypto and payment URI tests. A displayed payment QR code was decoded and matched its payment URI.

The complete legacy test suite is **not green**: known failures include old Bitcoin-specific fixtures, an expired payment-server test certificate, and wallet rescan/PSBT cases that still need investigation. Some Qt cases skip under the minimal macOS platform. This is experimental software, not a security audit or a claim of complete protocol compatibility.

Back up your wallet before experimenting. Wallet databases, private keys, local configuration, blockchain data and development-machine launchers are not included in this repository.

## Related project

[Machinecoin Android Wallet](https://github.com/amberhollow-ops/machinecoin-android-wallet)

## License and attribution

Machinecoin Core is derived from Bitcoin Core and Dash Core. The upstream history, copyright notices and [MIT license](COPYING) are retained. This is a community-maintained experimental adaptation, not an official upstream release.
