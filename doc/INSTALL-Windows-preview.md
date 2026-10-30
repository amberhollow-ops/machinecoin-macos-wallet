# Machinecoin Wallet — Windows x64 Midnight Preview

Native Windows x64 build of the community fork, based on `linux-preview`
commit `6969be6`. The Midnight interface and original Machinecoin application
icon are retained. This is an experimental community build.

The installer is English-only. On Windows the wallet defaults to English; an
explicit saved language preference or `-lang` option still takes precedence.

## Install and run

Run the installer. It installs for the current user under
`%LOCALAPPDATA%\Programs\Machinecoin-Midnight` and creates Start menu shortcuts.
Administrator rights are not needed. No wallet is launched automatically.
Qt and the required runtime DLLs are included; MSYS2 is not required to run it.

Normal starts use the existing Machinecoin data-directory selection behavior.
For an isolated preview, create a new directory and launch with explicit
`-datadir` and `-guisettingsdir` absolute paths. Do not point a preview test at
an existing wallet. An offline test example, after creating `C:\WalletTest`:

```powershell
& "$env:LOCALAPPDATA\Programs\Machinecoin-Midnight\machinecoin-qt.exe" `
  -datadir=C:\WalletTest -guisettingsdir=C:\WalletTest\gui `
  -regtest -connect=0 -listen=0 -dnsseed=0 -upnp=0 -splash=0
```

Regtest is an isolated test network; its coins have no value. The test flags
above disable peer connections. Mining remains opt-in.

## Remove

Use Windows Installed Apps or the Start menu uninstaller. Only the files
listed in the installed package are removed. Wallet databases, blockchain
data and GUI settings are not removed. No service, scheduled task, firewall
rule, autostart entry or URI handler is installed.

## Provenance

Source: https://github.com/amberhollow-ops/machinecoin-macos-wallet

The accompanying build report records the compiler, dependencies, changes
and tests. The executable and installer are unsigned. See COPYING and the
licenses directory for upstream and dependency licenses.

## Validation - preview.5

All six Qt suites pass (18 lifecycle/test results, zero failures or skips).
The three previous failures were corrected using runtime-generated test
certificates, Machinecoin-specific genesis/amount/PoW expectations, and a
correct UTF-8 marker for redacted RPC arguments. No validation was disabled.
The selected core tests pass (54 cases, 92,089 assertions).

Broader legacy core tests still fail: wallet tests expect Bitcoin block
rewards, and the PSBT fixture contains a Bitcoin private-key encoding that
fails Machinecoin decoding. The full core suite is therefore not green.
Mainnet synchronization, real-money transfers and a separate clean Windows
machine were not tested. The executable and installer remain unsigned.

The Midnight setup and uninstaller use a modern English WPF interface.
They require Windows .NET Framework 4.8 (included with Windows 11). The
embedded NSIS engine runs silently and preserves the exact removal manifest.


## Privacy rebuild candidate - preview.6

Application and dependency compilation remaps build paths. The validation above
describes preview.5; see VALIDATION.txt for the new candidate results.
