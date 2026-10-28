# Windows x64 Midnight build

## Source review

Requested fork: `amberhollow-ops/machinecoin-macos-wallet`, branch
`linux-preview`, revision `6969be6`.

The retained upstream `build-windows.md` describes Linux/WSL cross-compilation
using MinGW-w64 and names Ubuntu 18.04. It also lists MSYS2 as an alternative.
This build uses native MSYS2 MINGW64 GCC on Windows; the resulting executable
is a Windows PE x86-64 application, not a Linux executable or an MSYS program.

The branch contains a literal `19676 tokens truncated` marker in `src/init.cpp`.
Its commit replaces 1,446 lines of initialization/argument handling with that
marker. `src/init.cpp` was restored exactly from the same fork's parent commit
`27d62728db32ff9fc8f9f780cf9a65203dca556d`. The rest of the Linux preview changes
are retained. This restoration is necessary for any fresh build of the branch.

## Dependencies

| Component | Recorded build |
| --- | --- |
| GCC / MinGW-w64 | GCC 16.2.0, x86_64-w64-mingw32 |
| Qt | 5.15.19, dynamic runtime |
| Boost | 1.74.0, local static build |
| Berkeley DB | 4.8.30.NC, local static C++ build |
| Chia BLS / RELIC | codablock v20181101, local static build |
| Protobuf | 3.21.12, local static build and matching protoc |
| OpenSSL | 3.6.5 |
| libevent | 2.1.13 |
| GMP | 6.3.0 |
| libqrencode | 4.1.1 |
| MiniUPnPc | 2.3.3 |
| NSIS | 3.13 |

The repository's dependency table is historical (for example, it lists Qt
5.7.1 while the depends recipe uses 5.9.6). The MSYS2 Berkeley DB 6.2 and
Protobuf 36 packages were not substituted for the wallet's expected versions.
All four separately downloaded source archives are SHA-256 pinned by
`contrib/windows/bootstrap-deps.sh`.

The BLS reference BLAKE2 structs used incompatible packed/over-aligned typedefs
under modern GCC. The build removes their unnecessary alignment annotation;
the library's own known-answer and signature tests must pass afterwards.
No BLS algorithm, curve parameters or serialization format was replaced.

## Build

Use an MSYS2 MINGW64 shell with base-devel, Autotools, native GCC, CMake, Ninja,
Qt5 base/tools, OpenSSL, libevent, GMP, qrencode, miniupnpc and NSIS installed.
The helper scripts use `/mingw64/bin` before `/usr/bin`; CMake must be the native
MinGW build so response files contain Windows paths.

```sh
bash contrib/windows/bootstrap-deps.sh
bash contrib/windows/build-wallet.sh
```

The dependency prefix defaults to `../deps/prefix` relative to the repository.
Do not share configure caches across different compilers or dependency sets.
The GUI, wallet, QR, UPnP, daemon, command-line utilities and existing test
programs are enabled. D-Bus and ZeroMQ are disabled, as they are not required
by the wallet UI. UPnP is available but off by default.

Qt and selected runtime libraries remain dynamic; `LIBTOOL_APP_LDFLAGS=`
overrides the upstream Windows assumption that all libraries are static.
The package script recursively resolves DLL imports and rejects dependencies
on MSYS/Cygwin runtimes or unresolved DLLs.

## Windows changes

- Restore the truncated initialization source from its intact parent.
- Use amber navigation/status icons on Windows; retain original application
  icon assets and embedded Windows icon resources unchanged.
- Set Qt high-DPI attributes before QApplication construction.
- Support the current MiniUPnPc `UPNP_GetValidIGD` signature while preserving
  compatibility with older versions.
- Add optional `-guisettingsdir` for isolated INI preferences during tests.
  Normal starts preserve the existing settings behavior.
- Add native packaging and offline wallet smoke-test helpers.
- Use `gmtime_s` on MinGW as well as MSVC for UTC formatting.
- Repaint the mining page background when its live labels change, avoiding
  overlaid stale text on Windows.
- Allow `MACHINECOIN_QT_TEST_LOGDIR` to produce per-suite Qt test reports on
  Windows, where the GUI-subsystem test executable has no normal console.

## Package and test

From PowerShell, choose a new staging directory:

```powershell
./contrib/windows/package.ps1 -Stage C:\build\machinecoin-stage `
  -Output C:\build\Machinecoin-Midnight-Windows-x64-Setup.exe
```

Run `contrib/windows/test-wallet.py --bin <packaged-directory>
--tests-root <new-test-root>` with Python. It creates a unique regtest data
directory and verifies startup, address creation, message signing, backup,
encryption, restart, unlocking and clean shutdown with peer networking disabled.
For GUI testing, also pass `-guisettingsdir` to a directory inside the test root.

The NSIS `/TEST=1` switch suppresses shortcuts and Installed Apps registration
for packaging checks. `/D=<absolute-test-install-directory>` must be the final
installer argument. No wallet is launched by installation. The uninstaller
uses an exact file manifest and never recursively deletes an install or data
directory.

Build artifacts are local. No GitHub push, release, issue or pull request is
part of this workflow. Executables and installer are unsigned.

## Recorded validation â€” 28 October 2026

- Native PE machine type AMD64, Windows GUI subsystem for the wallet.
- Original icon resources are unchanged relative to the requested branch.
- BLS dependency: 8 test cases and 162 assertions passed.
- Core selection: `db_tests,wallet_crypto_tests,accounting_tests,coinselector_tests,crypto_tests,util_tests`;
  54 of 315 cases passed, 261 not selected, 92,089 assertions passed.
- Qt URI, address-book and compatibility suites passed. Three existing cases
  failed: PaymentServerTests uses a certificate expired in December 2022;
  RPCNestedTests expects the Bitcoin genesis hash; WalletTests expects 105
  transaction rows but observes 1. These failures also appear in the branch's
  previous Linux/macOS validation notes. They were not hidden or disabled.
- Offline regtest smoke tests passed both from the portable package and from
  an actual installer destination with PATH limited to Windows directories:
  101 generated blocks, spendable balance, sending, confirmed receipt,
  positive/negative message verification, backup, encryption, restart,
  unlocking, signing, relocking and clean shutdown.
- Interactive Windows 11 GUI checks: Midnight overview and balances, send
  form, payment request with displayed QR code, CPU mining start/stop and
  no automatic mining on restart. Live mining labels repaint correctly.
- Installer `/S /TEST=1` exited successfully; installed files matched staging
  hashes. Uninstall removed the program and preserved a synthetic `wallet.dat`
  sentinel placed in the test install directory.
- All wallet tests used new directories under the workspace's `tests` folder.
  GUI tests additionally used isolated INI settings under that directory.
  No existing wallet was opened and no mainnet transaction was made.

## Updated validation — preview.2, 28 October 2026

- PaymentServerTests now creates ephemeral, signed X.509 certificate chains
  relative to the execution date. Expired leaf/intermediate, future-dated,
  untrusted, empty-trust-store and tampered-signature cases remain negative.
  Amount boundaries use Machinecoin MAX_MONEY, not Bitcoin's supply limit.
  Test keys stay in memory; production certificate validation is unchanged.
- RPCNestedTests derives the expected transaction ID from Machinecoin genesis.
  A newly exposed production redaction-marker encoding bug is corrected with
  explicit UTF-8 bytes; sensitive argument filtering remains in place.
- TestChain100Setup mines using GetPoWHash and checks successful chain connection.
  Previously it used the block identifier, creating invalid fixture blocks.
  This is a test fixture correction, not a production consensus change.
- Qt test preferences use INI files inside the fresh temporary data directory.
- All six Qt suites pass: 18 results including setup/cleanup, zero failures,
  zero skips. Detailed reports are in validation/qt-suites.
- The original core selection passes again: 54 cases, 92,089 assertions.
- An expanded run including wallet_tests and psbt_wallet_tests fails: wallet
  assertions assume 50/100 COIN rather than Machinecoin's observed 45/90 COIN;
  the PSBT fixture decodes a Bitcoin WIF key and aborts at key.cpp:180.
  This diagnostic run is retained as preview2-core-tests.log. These separate
  legacy core fixtures were not altered as part of the three Qt fixes.

The full core suite is not green. This is not a security audit, mainnet
synchronization test, or validation on a separate clean Windows installation.
No files were published to GitHub. Midnight styling and original icons remain.

### Installer verification for preview.2

The rebuilt installer completed its isolated install/uninstall test. All 314
installed package files matched staging hashes. With PATH limited to Windows
system directories, the installed daemon passed mining, sending/confirmation,
message signing/verification, backup, encryption, restart, unlock and relock.
The installed native GUI started with separate data and INI settings, reported
regtest with zero peer connections, created an address and shut down via RPC.
Uninstall removed the program and preserved a synthetic wallet.dat sentinel.
All processes were shut down after the tests. No existing wallet was used.

Detailed current logs are included in the validation directory. BLS and visual
Midnight/QR checks described under preview.1 were not repeated in this update.

## English Midnight installer - preview.4

The stock installer wizard is replaced by a native WPF interface with Segoe UI
typography, dark panels, rounded controls, amber accents, a matching license
dialog, progress and completion screens. The maintenance interface uses the
same design. English is the only installer language; installer source text is
ASCII and the NSIS compiler input encoding is explicitly UTF-8.

The Windows wallet defaults to English. Explicit language preferences and
-lang remain supported. Original wallet icons and Midnight styling are kept.

The WPF executable embeds the NSIS payload and runs it silently. The same exact
file manifest, per-user installation and test isolation remain in place. The
installed maintenance launcher relocates itself to a unique temporary folder
before removal, allowing its installed copy to be removed too. Temporary setup
folders under TEMP contain the launcher or result/error logs, never wallet keys.
Setup refuses to change an installation whose wallet or daemon is running.
The UI requires Windows .NET Framework 4.8; no runtime is downloaded by setup.

Visual checks on German Windows verified the English wallet menus, the modern
setup layout and its matching license dialog. Earlier preview.2 core/Qt results
remain recorded above; no consensus or transaction behavior changed here.

Final preview.4 verification passed: 315 installed files matched staging hashes;
the installed daemon passed the isolated wallet smoke checks; the GUI started
and shut down cleanly; removal was refused while that GUI was running. After
shutdown, the modern maintenance launcher removed the application, including
both maintenance executables, and preserved the synthetic wallet.dat sentinel.
Tests used fresh offline regtest directories. No existing wallets were accessed.
No GitHub publication was performed. The installer and application are unsigned.

## Peer discovery repair - preview.5, 28 October 2026

The Linux launcher supplied 198.50.168.213:40333 and 66.70.182.1:40333 with
-addnode. Those addresses were missing from the Windows bootstrap list. Both
were independently verified from Windows with Octopus 0.17.1 / protocol 70025
handshakes and a mainnet height around 2,042,764.

Mainnet fixed seeds and their nodes_main.txt input now contain those two peers.
They enter normal address discovery, rather than being permanently pinned as
manual peers. The connection loop seeds an empty address manager and also
retries after 60 seconds without outbound connections when peers.dat contains
stale addresses. Its one-time state belongs to the connection thread, rather
than a process-static variable. Explicit -connect modes keep their existing
behavior; no wallet configuration or existing peer database is overwritten.
Testnet seed entries are unchanged. Existing DNS behavior is retained, although
the old DNS responses currently lead to unresponsive endpoints.

Observed baseline: preview.4 found zero peers and zero headers after 80 seconds.
With manual Linux nodes, it connected within four seconds and fetched headers.
Preview.5, without manual nodes, recovered both a fresh profile and the same
stale test profile in about 65 seconds, with a completed mainnet P2P handshake
and 2,000 headers. Tests stop after proving discovery; complete blockchain
synchronization and real-money transfers are not claimed.

Network regression selection: 26 cases, 8,973 assertions passed across
net_tests, netbase_tests and addrman_tests. All six Qt suites pass again,
18 results including lifecycle methods, zero failures and zero skips.

The packaged Windows GUI also found a mainnet peer without manual nodes in
68 seconds and downloaded 2,000 headers. A separate mainnet -connect=0 test
remained at zero peers and zero headers for 72 seconds, confirming isolation.
Final modern-installer verification passed: 315 files matched, offline wallet
functions passed, running-wallet removal was refused, and uninstall preserved
the synthetic wallet sentinel. All test nodes were stopped cleanly.
All tests used separate data directories. Existing user wallets were untouched.

To use this update, close the previous wallet, install preview.5 and reopen it
normally. Allow approximately one minute for bootstrap recovery if the old DNS
addresses time out. Do not delete wallet.dat or peers.dat. Full mainnet sync
continues after discovery and was not completed by these bounded tests.

## Peer retention verification

The Windows source already contains the Mac full-sync peer-retention fix from
27d6272. The masternode-sync.cpp Git blob matches the Mac master branch exactly
(6226dd9883eb9ed93afa8f3acbe674d598f22c5c). The dedicated cached-full-sync
regression test passes 10 assertions: peers stay connected, existing request
throttles remain effective, and the stage timeout still advances.

A separate test configuration persisted both addnode entries. The Windows GUI
retained both connections with unchanged IDs for three minutes, downloading
444,000 headers. After restarting with the saved configuration and without
new addnode arguments, both remained connected for another minute; headers
reached 783,999. No recently-synced-peer disconnect message appeared. These
are bounded initial-sync tests, not evidence of completed mainnet sync.

All tests used isolated directories. User wallet/configuration files were not
modified. Publication of source changes is a separate, explicitly authorized
step; installers, dependency binaries, test data and private keys are excluded
from the Git commit.
