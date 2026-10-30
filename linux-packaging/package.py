#!/usr/bin/env python3
"""Package a built source tree. Never include local configure logs or build directories."""
from pathlib import Path
import os,subprocess,shutil,tarfile
source=Path(__file__).resolve().parent.parent
build=Path(os.environ.get('MACHINECOIN_BUILD',str(source))).resolve()
out=Path(os.environ.get('MACHINECOIN_PACKAGE_OUT',str(source.parent/'release-output'))).resolve()
out.mkdir(parents=True,exist_ok=True)
stage=out/'stage'
if stage.exists(): raise SystemExit('Choose a fresh output directory; stage already exists')
def copy(src,name):
 p=stage/name;p.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,p)
def write(name,text,mode=0o644):
 p=stage/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(text);p.chmod(mode)
copy(build/'src/qt/machinecoin-qt','usr/lib/machinecoin-wallet/machinecoin-qt')
write('usr/bin/machinecoin-wallet','#!/bin/sh\nexec /usr/lib/machinecoin-wallet/machinecoin-qt "$@"\n',0o755)
copy(source/'src/qt/res/icons/machinecoin.png','usr/share/icons/hicolor/1024x1024/apps/machinecoin.png')
copy(source/'COPYING','usr/share/doc/machinecoin-wallet-experimental/copyright')
copy(source/'linux-packaging/INSTALL-Linux.md','usr/share/doc/machinecoin-wallet-experimental/INSTALL-Linux.md')
deps=Path(os.environ.get('MACHINECOIN_DEPS_SOURCE',str(build/'local-deps')))
for src,name in [(deps/'db-4.8.30.NC/LICENSE','BerkeleyDB'),(deps/'boost_1_74_0/LICENSE_1_0.txt','Boost'),(deps/'bls-signatures/LICENSE','BLS'),(source/'src/leveldb/LICENSE','LevelDB'),(source/'src/secp256k1/COPYING','secp256k1'),(source/'src/univalue/COPYING','UniValue'),(source/'src/crypto/ctaes/COPYING','ctaes')]:
 copy(src,'usr/share/doc/machinecoin-wallet-experimental/licenses/'+name)
write('usr/share/applications/machinecoin-wallet.desktop','[Desktop Entry]\nType=Application\nName=Machinecoin Wallet (Experimental)\nComment=Experimental Machinecoin desktop wallet\nExec=machinecoin-wallet %u\nIcon=machinecoin\nTerminal=false\nCategories=Office;Finance;\nStartupWMClass=Machinecoin-qt\n')
(out/'debian').mkdir()
(out/'debian/control').write_text('Source: machinecoin-wallet-experimental\nSection: utils\nPriority: optional\nMaintainer: amberhollow-ops <noreply@github.com>\n\nPackage: machinecoin-wallet-experimental\nArchitecture: amd64\nDescription: Experimental Machinecoin wallet\n')
dep=subprocess.check_output(['dpkg-shlibdeps','-O',str(stage/'usr/lib/machinecoin-wallet/machinecoin-qt')],cwd=out,text=True).strip().split('=',1)[1]
write('DEBIAN/control',f'Package: machinecoin-wallet-experimental\nVersion: 0.17.1~preview.2+ubuntu26.04\nArchitecture: amd64\nMaintainer: amberhollow-ops <noreply@github.com>\nSection: utils\nPriority: optional\nDepends: {dep}, libqt5gui5t64 (>= 5.15.0)\nHomepage: https://github.com/amberhollow-ops/machinecoin-macos-wallet\nDescription: Experimental Machinecoin Midnight desktop wallet\n Privacy rebuild for Ubuntu 26.04 amd64. Community preview.\n')
subprocess.run(['dpkg-deb','--root-owner-group','--build',str(stage),str(out/'Machinecoin-Wallet-0.17.1-preview.2-ubuntu26.04-amd64.deb')],check=True)
# The manifest is produced from the clean release source snapshot, not the build tree.
with tarfile.open(out/'Machinecoin-Linux-preview.2-source.tar.gz','w:gz') as tar:
 for rel in (source/'linux-packaging/source-files.txt').read_text().splitlines():
  p=source/rel
  if p.is_symlink() or not p.is_file() or '..' in Path(rel).parts:raise SystemExit('Unsafe source manifest entry')
  info=tar.gettarinfo(str(p),arcname='machinecoin-linux-source/'+rel);info.uid=info.gid=0;info.uname=info.gname='root'
  with p.open('rb') as f:tar.addfile(info,f)
shutil.copy2(source/'linux-packaging/INSTALL-Linux.md',out/'INSTALL-Linux.md')
