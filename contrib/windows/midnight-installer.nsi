Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
!ifndef STAGE
  !error "Pass /DSTAGE=<absolute directory containing the packaged application>"
!endif
!ifndef OUTPUT
  !error "Pass /DOUTPUT=<installer output path>"
!endif
Name "Machinecoin Wallet - Midnight Preview"
OutFile "${OUTPUT}"
InstallDir "$LOCALAPPDATA\Programs\Machinecoin-Midnight"
RequestExecutionLevel user
SetCompressor /SOLID lzma
VIProductVersion "0.17.1.5"
VIAddVersionKey /LANG=1033 "ProductName" "Machinecoin Wallet Midnight Preview"
VIAddVersionKey /LANG=1033 "FileDescription" "Machinecoin Wallet Windows x64 installer"
VIAddVersionKey /LANG=1033 "FileVersion" "0.17.1-preview.5"
VIAddVersionKey /LANG=1033 "LegalCopyright" "Machinecoin Core contributors"
!define MUI_ICON "..\..\src\qt\res\icons\machinecoin.ico"
!define MUI_UNICON "..\..\src\qt\res\icons\machinecoin.ico"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Var TestInstall
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This build requires 64-bit Windows."
    Abort
  ${EndIf}
  ${GetParameters} $R0
  ClearErrors
  ${GetOptions} $R0 "/TEST=" $TestInstall
FunctionEnd
Section "Wallet"
  SetOutPath "$INSTDIR"
  File /r "${STAGE}\*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  ${If} $TestInstall == "1"
    FileOpen $R0 "$INSTDIR\test-install.marker" w
    FileClose $R0
  ${Else}
    CreateDirectory "$SMPROGRAMS\Machinecoin Midnight"
    CreateShortcut "$SMPROGRAMS\Machinecoin Midnight\Machinecoin Wallet.lnk" "$INSTDIR\machinecoin-qt.exe"
    CreateShortcut "$SMPROGRAMS\Machinecoin Midnight\Uninstall.lnk" "$INSTDIR\MachinecoinMaintenance.exe" "--uninstall"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview" "DisplayName" "Machinecoin Wallet - Midnight Preview (x64)"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview" "UninstallString" '$\"$INSTDIR\MachinecoinMaintenance.exe$\" --uninstall'
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview" "DisplayIcon" "$INSTDIR\machinecoin-qt.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview" "DisplayVersion" "0.17.1-preview.5"
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview" "NoModify" 1
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview" "NoRepair" 1
  ${EndIf}
SectionEnd
Section "Uninstall"
  IfFileExists "$INSTDIR\test-install.marker" skip_registration
    Delete "$SMPROGRAMS\Machinecoin Midnight\Machinecoin Wallet.lnk"
    Delete "$SMPROGRAMS\Machinecoin Midnight\Uninstall.lnk"
    RMDir "$SMPROGRAMS\Machinecoin Midnight"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MachinecoinMidnightPreview"
  skip_registration:
  ; Generated exact file list: never recursively delete an install or data directory.
  !include "${STAGE}\uninstall-files.nsh"
  Delete "$INSTDIR\test-install.marker"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
SectionEnd
