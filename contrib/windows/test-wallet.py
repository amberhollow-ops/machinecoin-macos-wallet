"""Offline regtest smoke test. Always creates a new data directory."""
import argparse
import json
import os
from pathlib import Path
import socket
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument("--bin", required=True, type=Path)
parser.add_argument("--tests-root", required=True, type=Path)
args = parser.parse_args()
args.tests_root.mkdir(parents=True, exist_ok=True)
data = Path(tempfile.mkdtemp(prefix="wallet-regtest-", dir=args.tests_root.resolve()))
with socket.socket() as sock:
    sock.bind(("127.0.0.1", 0))
    port = sock.getsockname()[1]
common = [f"-datadir={data}", "-regtest", f"-rpcport={port}"]
cli = str(args.bin.resolve() / "machinecoin-cli.exe")
daemon = str(args.bin.resolve() / "machinecoind.exe")
checks = []
process = None
log = open(data / "process.log", "w")

def rpc(*params):
    result = subprocess.run([cli, *common, *map(str, params)], capture_output=True, text=True, timeout=60)
    if result.returncode:
        raise RuntimeError(result.stderr.strip())
    value = result.stdout.strip()
    try:
        return json.loads(value)
    except ValueError:
        return value

def start():
    global process
    process = subprocess.Popen([daemon, *common, "-server=1", "-rpcbind=127.0.0.1", "-listen=0", "-connect=0", "-dnsseed=0", "-discover=0", "-upnp=0", "-listenonion=0", "-keypool=10"], stdout=log, stderr=log)
    for _ in range(120):
        if process.poll() is not None:
            raise RuntimeError(f"Node exited with {process.returncode}; see {data}")
        try:
            rpc("getwalletinfo")
            return
        except RuntimeError:
            time.sleep(0.5)
    raise RuntimeError("RPC startup timed out")

try:
    start()
    assert rpc("getblockchaininfo")["chain"] == "regtest"
    assert rpc("getconnectioncount") == 0
    checks.append("isolated offline regtest startup")
    address = rpc("getnewaddress", "Windows smoke test", "legacy")
    assert rpc("validateaddress", address)["isvalid"]
    signature = rpc("signmessage", address, "Machinecoin Windows x64 smoke test")
    assert rpc("verifymessage", address, signature, "Machinecoin Windows x64 smoke test") is True
    assert rpc("verifymessage", address, signature, "different message") is False
    checks.append("address creation and message signing/verification")
    # Exercise mining and sending using valueless coins on the isolated chain.
    blocks = rpc("generatetoaddress", 101, address)
    assert len(blocks) == 101
    assert float(rpc("getbalance")) > 0
    recipient = rpc("getnewaddress", "regtest recipient", "legacy")
    rpc("settxfee", "0.001")
    txid = rpc("sendtoaddress", recipient, "1.0")
    assert len(txid) == 64
    assert len(rpc("generatetoaddress", 1, address)) == 1
    assert rpc("gettransaction", txid)["confirmations"] >= 1
    checks.append("regtest mining, spendable balance, send and confirmed receipt")
    backup = data / "wallet-backup.dat"
    rpc("backupwallet", backup)
    assert backup.is_file() and backup.stat().st_size > 0
    checks.append("wallet backup")
    rpc("encryptwallet", "regtest-only-never-use-for-real-wallets")
    rpc("stop")
    process.wait(timeout=60)
    start()
    rpc("walletpassphrase", "regtest-only-never-use-for-real-wallets", 30)
    signature2 = rpc("signmessage", address, "encrypted-wallet-restart")
    assert rpc("verifymessage", address, signature2, "encrypted-wallet-restart") is True
    rpc("walletlock")
    assert rpc("getwalletinfo")["unlocked_until"] == 0
    checks.append("encryption, clean restart, unlock, signing, relock")
    rpc("stop")
    assert process.wait(timeout=60) == 0
    checks.append("clean shutdown")
    report = {"success": True, "data_directory": str(data), "checks": checks}
    (data / "result.json").write_text(json.dumps(report, indent=2))
    print(json.dumps(report, indent=2))
finally:
    if process is not None and process.poll() is None:
        try:
            rpc("stop")
            process.wait(timeout=30)
        except Exception:
            process.terminate()
            process.wait(timeout=10)
    log.close()
